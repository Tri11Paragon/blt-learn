/*
 *  <Short Description>
 *  Copyright (C) 2026  Brett Terpstra
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <blt/std/thread.h>
#include <blt/iterator/iterator.h>

#include <blt/std/random.h>
#include <blt/std/assert.h>
#include <net/socket.h>

inline volatile std::atomic_bool should_stop = false;

template <typename Model, typename Federate>
struct client_t
{
    client_t(Federate& fed, const double process_time, const double sync_delay) : process_time(process_time),
        sync_delay(sync_delay), federator(&fed)
    {
        th = std::thread{
            [](client_t* client)
            {
                client->thread_ready = true;
                while (!should_stop)
                {
                    std::unique_lock<std::mutex> lock(client->mut);
                    while (!client->ready)
                        client->cv.wait(lock);
                    client->ready = false;
                    // std::this_thread::sleep_for(
                    // std::chrono::microseconds(static_cast<long>(client.process_time * 1000)));
                    client->local_model.process();
                    // std::this_thread::sleep_for(
                    // std::chrono::microseconds(static_cast<long>(client.sync_delay * 1000)));
                    client->federator->federate(client->local_model);
                }
            },
            this
        };
    }

    client_t(client_t&&) = default;
    client_t& operator=(client_t&&) = default;

    void process()
    {
        ready = true;
        cv.notify_all();
    }

    void update(const Model& model)
    {
        local_model = model;
    }

    double process_time;
    double sync_delay;
    std::mutex mut;
    std::condition_variable cv;
    std::thread th;
    Federate* federator;
    std::atomic_bool ready = false;
    std::atomic_bool thread_ready = false;
    Model local_model;
};

template <typename Model>
struct server_t
{
    using Client = client_t<Model, server_t>;

    explicit server_t(const blt::u32 per_round_clients) : per_round_clients(per_round_clients)
    {
        start_thread();
    }

    template <typename ClientPred>
    explicit server_t(const blt::u32 per_round_clients, const blt::size_t num_clients,
                      ClientPred pred) : per_round_clients(per_round_clients)
    {
        for (size_t i = 0; i < num_clients; i++)
            clients.emplace_back(pred());
        start_thread();
        await_clients();
    }

    void add_client(std::unique_ptr<Client> client)
    {
        clients.emplace_back(std::move(client));
    }

    void await_clients()
    {
        while (blt::iterate(clients).map([](const auto& client)
        {
            return !client->thread_ready;
        }).any())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void process()
    {
        ready = true;
        round_finished = false;
        ready_cv.notify_all();
    }

    void federate(const Model& model)
    {
        std::scoped_lock lock(model_mut);
        model_queue.emplace_back(model);
        federate_cv.notify_all();
    }

    void await_finish()
    {
        std::unique_lock data{model_mut};
        while (!round_finished)
            finished_cv.wait(data);
        round_finished = false;
    }

    blt::u32 per_round_clients;

    std::vector<std::unique_ptr<Client>> clients;
    std::vector<Model> model_queue;
    std::mutex model_mut;

    std::thread th;
    std::condition_variable ready_cv;
    std::condition_variable federate_cv;
    std::condition_variable finished_cv;
    std::atomic_bool ready = false;
    std::atomic_bool round_finished = false;
    std::atomic_bool thread_ready = false;
    Model server_model;

private:
    void start_thread()
    {
        th = std::thread{
            [](server_t* srv)
            {
                blt::random::random_t random{std::random_device()()};
                auto& server = *srv;
                server.thread_ready = true;
                while (!should_stop)
                {
                    std::vector<blt::size_t> clients_to_run;
                    {
                        std::unique_lock<std::mutex> lock(server.model_mut);
                        while (!server.ready)
                            server.ready_cv.wait(lock);
                        server.ready = false;

                        BLT_TRACE("Server Starting Round");
                        BLT_ASSERT(server.per_round_clients <= server.clients.size());
                        for (size_t i = 0; i < server.clients.size(); i++)
                            clients_to_run.push_back(i);
                        std::shuffle(clients_to_run.begin(), clients_to_run.end(), random);
                        for (auto client : blt::iterate(clients_to_run).take(server.per_round_clients))
                            server.clients[client]->process();
                    }
                    BLT_TRACE("Awaiting clients");
                    std::unique_lock<std::mutex> lock(server.model_mut);
                    do
                    {
                        server.federate_cv.wait(lock);
                        if (server.model_queue.size() == server.per_round_clients)
                            break;
                    }
                    while (true);
                    BLT_TRACE("Aggregating models");
                    server.server_model.aggregate(server.model_queue);
                    server.model_queue.clear();
                    for (auto client : blt::iterate(clients_to_run).take(server.per_round_clients))
                        server.clients[client]->update(server.server_model);
                    BLT_TRACE("Finished!");
                    server.round_finished = true;
                    server.finished_cv.notify_all();
                }
            },
            this
        };
        while (!thread_ready)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
};

#include <logistic.h>
#include <mnist_loader.h>

constexpr bool even_dist = true;
constexpr double digit_1 = 0;
constexpr double digit_2 = 5;
constexpr blt::u32 num_clients = 30;
constexpr blt::u32 per_round_clients = 10;
constexpr blt::u32 local_iterations = 70;
constexpr double target_loss = 0.4;
constexpr double min_process_time = 10;
constexpr double max_process_time = 80;
constexpr double min_sync_delay = 10;
constexpr double max_sync_delay = 80;

// manually set otherwise will generate
std::vector<double> client_distributions_digit1;
std::vector<double> client_distributions_digit2;

struct model_t
{
    std::vector<std::vector<double>> digits;
    std::vector<double> labels;
    blt::binary_regression_t regressor{784, 1e-3};
    double local_loss = 0;

    void process()
    {
        blt::random::random_t random{std::random_device()()};
        for (blt::u32 i = 0; i < local_iterations; i++)
        {
            const auto index = random.get_size_t(0, labels.size());
            local_loss += regressor.train_once(digits[index], labels[index]);
        }
        local_loss /= local_iterations;
    }

    void aggregate(const std::vector<model_t>& models)
    {
        const auto min = std::min_element(models.begin(), models.end(), [](const auto& a, const auto& b)
        {
            return a.local_loss < b.local_loss;
        });
        regressor.model = min->regressor.model;
        local_loss = min->local_loss;
    }
};

void populate_dist(std::vector<double>& dist)
{
    blt::random::random_t random{std::random_device()()};
    // randomly distribute for non I.I.D data
    double left = 1;
    if constexpr (!even_dist)
    {
        for (auto& i : dist)
        {
            const auto v = random.get_double(0, left);
            i = v;
            left -= v;
        }
    }
    // even on what's left
    const auto per = left / static_cast<double>(dist.size());
    for (double& i : dist)
        i += per;
}

std::vector<double> normal_form(const std::vector<double>& dist)
{
    std::vector<double> normalized_dist;
    double total = 0;
    for (const double client_distribution : dist)
    {
        total += client_distribution;
        normalized_dist.emplace_back(total);
    }
    for (auto& d : normalized_dist)
        d /= total;
    return normalized_dist;
}

int main()
{
    const blt::fs::idx_file_t images{"../train-images.idx3-ubyte"};
    const blt::fs::idx_file_t labels{"../train-labels.idx1-ubyte"};

    const auto data = images.cast_to<blt::u8, double>();
    auto lbls = labels.cast_to<blt::u8, double>();

    std::vector<std::vector<double>> image_data;
    const auto& sizes = images.get_dimensions();
    const auto image_size = sizes[1] * sizes[2];
    const auto image_count = sizes[0];

    for (size_t i = 0; i < image_count; i++)
    {
        image_data.emplace_back();
        image_data.back().resize(image_size);
        std::memcpy(image_data.back().data(), data.data() + i * image_size, image_size * sizeof(double));
    }

    std::vector<std::vector<double>> digit_1_data;
    std::vector<std::vector<double>> digit_2_data;
    std::vector<double> digit_1_labels;
    std::vector<double> digit_2_labels;

    for (const auto& [image, label] : blt::zip(image_data, lbls))
    {
        if (label == digit_1)
        {
            digit_1_data.emplace_back(image);
            digit_1_labels.emplace_back(0);
        }
        else if (label == digit_2)
        {
            digit_2_data.emplace_back(image);
            digit_2_labels.emplace_back(1);
        }
    }


    if (client_distributions_digit1.empty())
    {
        client_distributions_digit1.resize(num_clients);
        populate_dist(client_distributions_digit1);
    }

    if (client_distributions_digit2.empty())
    {
        client_distributions_digit2.resize(num_clients);
        populate_dist(client_distributions_digit2);
    }

    auto d1_normals = normal_form(client_distributions_digit1);
    auto d2_normals = normal_form(client_distributions_digit2);

    blt::random::random_t random{std::random_device()()};
    server_t<model_t> server{per_round_clients};
    for (blt::u32 i = 0; i < num_clients; i++)
    {
        auto client = std::make_unique<server_t<model_t>::Client>(server,
                                                                  random.get_double(min_process_time, max_process_time),
                                                                  random.get_double(min_sync_delay, max_sync_delay));
        server.add_client(std::move(client));
    }
    server.await_clients();

    while (!digit_1_labels.empty())
    {
        auto value = random.get_double();
        blt::u32 client = 0;
        for (blt::u32 i = 0; i < num_clients; i++)
        {
            if (value < d1_normals[i])
            {
                client = i;
                break;
            }
        }
        auto& model = server.clients[client]->local_model;
        model.digits.emplace_back(std::move(digit_1_data.back()));
        model.labels.emplace_back(digit_1_labels.back());
        digit_1_data.pop_back();
        digit_1_labels.pop_back();
    }

    while (!digit_2_labels.empty())
    {
        auto value = random.get_double();
        blt::u32 client = 0;
        for (blt::u32 i = 0; i < num_clients; i++)
        {
            if (value < d2_normals[i])
            {
                client = i;
                break;
            }
        }
        auto& model = server.clients[client]->local_model;
        model.digits.emplace_back(digit_2_data.back());
        model.labels.emplace_back(digit_2_labels.back());
        digit_2_data.pop_back();
        digit_2_labels.pop_back();
    }

    for (const auto& [i, client] : blt::enumerate(server.clients))
    {
        BLT_INFO("Client {}: {} images", i, client->local_model.digits.size());
        blt::size_t count = 0;
        for (const auto& d : client->local_model.labels)
            count += (d == 0);
        BLT_INFO("\tDigit 1: {} images | Digit 2: {}", count, client->local_model.labels.size() - count);
    }

    do
    {
        server.process();
        server.await_finish();
        BLT_TRACE("Round completed with loss {}", server.server_model.local_loss);
    }
    while (server.server_model.local_loss > target_loss);

    should_stop = true;
    std::exit(0);
}
