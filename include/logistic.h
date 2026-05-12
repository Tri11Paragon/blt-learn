#pragma once
/*
 *  Copyright (C) 2024  Brett Terpstra
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

#ifndef BLT_LOGISTIC_H
#define BLT_LOGISTIC_H

#include <blt/math/vectors.h>
#include <blt/math/math.h>
#include <blt/std/random.h>
#include <cmath>
#include <iostream>
#include <utility>

// adapted from https://medium.com/@sroy10012001/building-logistic-regression-from-scratch-in-c-9b68ab1ff5b8
namespace blt
{
    struct logistic_model_t
    {
        std::vector<double> weights;
        double bias;

        logistic_model_t(std::vector<double> weights, const double bias) : weights(std::move(weights)), bias(bias)
        {
        }

        explicit logistic_model_t(const size_t size) : bias(0)
        {
            weights.resize(size, 0);
        }

        logistic_model_t& operator+=(const logistic_model_t& other)
        {
            for (size_t i = 0; i < weights.size(); i++)
                weights[i] += other.weights[i];
            bias += other.bias;
            return *this;
        }

        template <typename T>
        friend logistic_model_t operator*(T value, const logistic_model_t& other)
        {
            logistic_model_t ret = other;
            for (double& weight : ret.weights)
                weight *= value;
            ret.bias *= value;
            return ret;
        }

        template <typename T>
        friend logistic_model_t operator*(const logistic_model_t& other, T value)
        {
            return value * other;
        }

        template <typename T>
        logistic_model_t& operator/=(T amount)
        {
            for (double& weight : weights)
                weight /= static_cast<double>(amount);
            bias /= static_cast<double>(amount);
            return *this;
        }

        static double dot(const std::vector<double>& a, const std::vector<double>& b)
        {
            double result = 0.0;
            for (size_t i = 0; i < a.size(); i++)
            {
                result += a[i] * b[i];
            }
            return result;
        }

        static double sigmoid(const double z)
        {
            return 1.0 / (1.0 + std::exp(-z));
        }

        template <typename T>
        [[nodiscard]] double classify(const std::vector<T>& x) const
        {
            const double z = dot(weights, x) + bias;
            return sigmoid(z);
        }

        template <typename T>
        [[nodiscard]] double predict(const std::vector<T>& x) const
        {
            return classify(x) > 0.5;
        }
    };

    struct binary_regression_t
    {
        struct test_result_t
        {
            double digit1_correct;
            double digit1_incorrect;
            double digit2_correct;
            double digit2_incorrect;

            [[nodiscard]] double digit_1_total() const
            {
                return digit1_correct + digit1_incorrect;
            }

            [[nodiscard]] double digit_1_accuracy() const
            {
                return digit1_correct / digit_1_total();
            }

            [[nodiscard]] double digit_2_total() const
            {
                return digit2_correct + digit2_incorrect;
            }

            [[nodiscard]] double digit_2_accuracy() const
            {
                return digit2_correct / digit_2_total();
            }

            [[nodiscard]] double total() const
            {
                return digit_1_total() + digit_2_total();
            }

            [[nodiscard]] double overall_accuracy() const
            {
                return (digit1_correct + digit2_correct) / total();
            }
        };

        struct test_data_t
        {
            // digit1 must be trained as 0
            std::vector<std::vector<double>> digit1;
            // digit2 must be trained as 1
            std::vector<std::vector<double>> digit2;
        };

        binary_regression_t(const int input_dim, const double lr, const int batch_size = 1)
            : model(input_dim), d_weights(input_dim, 0.0), lr(lr), batch_size(batch_size)
        {
        }

        explicit binary_regression_t(logistic_model_t m, const double lr = 0,
                                     const int batch_size = 128) : model(std::move(m)), lr(lr),
                                                                   batch_size(batch_size)
        {
        }

        void compute_gradient(const std::vector<double>& x, const double y, const double y_pred)
        {
            const auto l_error = y_pred - y;
            for (size_t i = 0; i < x.size(); i++)
            {
                d_weights[i] += l_error * x[i];
            }
            d_error += l_error;
        }

        void update_parameters()
        {
            for (size_t i = 0; i < model.weights.size(); i++)
            {
                model.weights[i] -= lr * d_weights[i];
            }
            model.bias -= lr * d_error;
        }

        static double binary_cross_entropy(const double y_true, double y_pred)
        {
            y_pred = std::max(std::min(y_pred, 1.0 - EPSILON), static_cast<double>(EPSILON));
            return -(y_true * std::log(y_pred) + (1.0 - y_true) * std::log(1 - y_pred));
        }

        double train_once(const std::vector<double>& image, const double label)
        {
            zero_grad();
            const double y_pred = model.predict(image);
            const auto loss = binary_cross_entropy(label, y_pred);
            compute_gradient(image, label, y_pred);
            update_parameters();
            return loss;
        }

        double train(const std::vector<std::vector<double>>& data, const std::vector<double>& labels, const int epochs)
        {
            BLT_TRACE("Training with learn rate {}", lr);
            std::vector<size_t> indices(data.size());
            for (size_t i = 0; i < indices.size(); i++) indices[i] = i;
            double total_loss = 0.0;
            for (int epoch = 0; epoch < epochs; epoch++)
            {
                double local_loss = 0.0;
                std::shuffle(indices.begin(), indices.end(), random);
                const size_t end = std::min(static_cast<size_t>(batch_size), data.size());
                zero_grad();
                for (size_t i = 0; i < end; i++)
                {
                    const size_t idx = indices[i];
                    const double y_pred = model.predict(data[idx]);
                    local_loss += binary_cross_entropy(labels[idx], y_pred);
                    compute_gradient(data[idx], labels[idx], y_pred);
                }
                // Average gradients
                const size_t current_batch_size = end;
                for (auto& d : d_weights)
                    d /= static_cast<double>(current_batch_size);
                d_error /= static_cast<double>(current_batch_size);
                update_parameters();
                total_loss += local_loss / static_cast<double>(data.size());
            }
            return total_loss / epochs;
        }

        [[nodiscard]] test_result_t test(const test_data_t& testing_data) const
        {
            double digit1_correct = 0;
            double digit1_incorrect = 0;
            double digit2_correct = 0;
            double digit2_incorrect = 0;
            for (const auto& d1 : testing_data.digit1)
            {
                const auto classify_d1 = model.classify(d1);
                if (classify_d1 < 0.5)
                    digit1_correct++;
                else
                    digit1_incorrect++;
            }
            for (const auto& d2 : testing_data.digit2)
            {
                const auto classify_d2 = model.classify(d2);
                if (classify_d2 > 0.5)
                    digit2_correct++;
                else
                    digit2_incorrect++;
            }

            return {
                digit1_correct,
                digit1_incorrect,
                digit2_correct,
                digit2_incorrect
            };
        }


        void zero_grad()
        {
            d_error = 0;
            for (auto& w : d_weights)
                w = 0.0;
        }

        [[nodiscard]] logistic_model_t get_weights() const { return model; }

        random::random_t random{std::random_device{}()};
        logistic_model_t model;
        std::vector<double> d_weights;
        double d_error = 0;
        double lr;
        int batch_size;
    };
}


#endif
