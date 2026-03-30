import javax.swing.*;
import java.awt.*;
import java.io.IOException;
import java.lang.classfile.*;
import java.lang.classfile.attribute.CodeAttribute;
import java.lang.classfile.instruction.NewObjectInstruction;
import java.lang.constant.ClassDesc;
import java.lang.reflect.AccessFlag;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.HashSet;
import java.util.stream.Collectors;

public class Run1P03 {

    public static void main(String[] args) throws IOException {
        if (args.length < 3)
        {
            System.out.println("Expected (file) (brock_jar) (class_path)");
            return;
        }
        var path = Path.of(args[0]);
        java.net.URL[] urls = new java.net.URL[args.length - 1];
        for (int i = 1; i < args.length; i++)
            urls[i - 1] = Path.of(args[i]).toUri().toURL();

        for (var url : urls) {
            System.out.println("Adding URL: " + url);
        }

        var bytes = Files.readAllBytes(path);
        var clazz = ClassFile.of().parse(bytes);
//        for (var field : clazz.fields()) {
//            System.out.println("Field: " + flags(field.flags()) + field.fieldTypeSymbol().displayName() + " " + field.fieldName().stringValue());
//        }

        for (var method : clazz.methods()) {
            var type = method.methodTypeSymbol();
            System.out.print("Method: " + flags(method.flags()) + type.returnType().displayName() + " " + method.methodName().stringValue() + "(");
            System.out.print(Arrays.stream(type.parameterArray()).map(ClassDesc::displayName).collect(Collectors.joining(", ")));
            System.out.println(")");
            if (!method.methodName().stringValue().equals("main"))
                continue;
            var codeAttrOpt = method.findAttribute(Attributes.code());
            if (codeAttrOpt.isEmpty()) {
                System.out.println("No code for method.");
                continue;
            }
            execute(clazz, bytes, urls);
            return;
        }
        System.out.println("No main method found in class " + clazz.thisClass().name());
    }

    public static void execute(ClassModel clazz, byte[] bytes, java.net.URL[] urls){
        for (var url : urls){
            System.out.println("URL: " + url);
        }
        // Compute the binary name (e.g., "pkg.Foo") from internal name (e.g., "pkg/Foo")
        String internalName = String.valueOf(clazz.thisClass().name());
        String binaryName = internalName.replace('/', '.');

        var parent = new java.net.URLClassLoader(urls, ClassLoader.getSystemClassLoader());

        // Load the class bytes into an isolated loader to avoid linkage conflicts
        var loader = new BytesClassLoader(parent);

        try {
            Class<?> target = loader.define(binaryName, bytes);
            // Get main(String[]) and forward remaining CLI args (after the .class path)
            var m = target.getMethod("main", String[].class);

            System.out.println("Invoking " + binaryName + ".main");
            wait_for_frame_close();
            m.invoke(null, (Object) new String[0]);
        } catch (Throwable t) {
            System.out.println("Failed to invoke main: " + t);
            System.out.println("On file: " + binaryName);
            t.printStackTrace();
        }
    }

    static void wait_for_frame_close() {
        new Thread(() -> {
            System.out.println("Checking to make sure threads exit.");
            HashSet<Frame> active_frames = new HashSet<>();
            while (true) {
                var frames = JFrame.getFrames();
                for (var frame : frames) {
                    if (frame.getTitle() == null)
                        continue;
                    if (frame.getTitle().isEmpty())
                        continue;
                    if (!frame.isVisible())
                        continue;
                    active_frames.add(frame);
                }
                for (var frame : active_frames) {
                    if (!frame.isVisible()) {
                        System.out.println(frame.getTitle() + " is no longer visible.");
                        System.exit(0);
                    }
                }
                try {
                    Thread.sleep(50);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }
        }, "Validator").start();
    }

    static String flags(AccessFlags flags) {
        String str = " ";
        if (flags.has(AccessFlag.PUBLIC)) {
            str += "public ";
        }
        if (flags.has(AccessFlag.PRIVATE)) {
            str += "private ";
        }
        if (flags.has(AccessFlag.STATIC)) {
            str += "static ";
        }
        if (flags.has(AccessFlag.FINAL)) {
            str += "final ";
        }
        if (flags.has(AccessFlag.VOLATILE)) {
            str += "volatile ";
        }
        if (flags.has(AccessFlag.ABSTRACT)) {
            str += "abstract ";
        }
        if (flags.has(AccessFlag.SYNCHRONIZED)) {
            str += "synchronized ";
        }
        if (flags.has(AccessFlag.TRANSITIVE)) {
            str += "transitive ";
        }
        return str;
    }

    static final class BytesClassLoader extends ClassLoader {
        BytesClassLoader(ClassLoader parent) {
            super(parent);
        }

        Class<?> define(String binaryName, byte[] bytes) {
            return defineClass(binaryName, bytes, 0, bytes.length);
        }
    }


}
