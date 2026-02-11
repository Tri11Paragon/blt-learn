import javax.swing.*;
import java.awt.*;
import java.io.IOException;
import java.lang.classfile.AccessFlags;
import java.lang.classfile.Attributes;
import java.lang.classfile.ClassFile;
import java.lang.classfile.MethodModel;
import java.lang.classfile.attribute.CodeAttribute;
import java.lang.classfile.instruction.NewObjectInstruction;
import java.lang.constant.ClassDesc;
import java.lang.reflect.AccessFlag;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.HashSet;
import java.util.stream.Collectors;
import java.util.ArrayList;

public class RunBlueJ {

    public static void main(String[] args) throws IOException {
        Path brock_jar = Path.of(args[0]);
        var path = Path.of(args[1]);
        if (!Files.exists(path)) {
            System.out.println("File not found: " + path);
            return;
        }
        if (!path.toString().endsWith(".class")) {
            System.out.println("Not a class file: " + path);
            return;
        }

        java.net.URL[] urls = new java.net.URL[args.length > 2 ? 3 : 2];
        urls[0] = brock_jar.toUri().toURL();
        urls[1] = path.getParent().toUri().toURL();
        if (args.length > 2)
            urls[2] = Path.of(args[2]).toUri().toURL();

        for (var url : urls) {
            System.out.println("Adding URL: " + url);
        }

        var bytes = Files.readAllBytes(path);
        var clazz = ClassFile.of().parse(bytes);
        for (var field : clazz.fields()) {
            System.out.println("Field: " + flags(field.flags()) + field.fieldTypeSymbol().displayName() + " " + field.fieldName().stringValue());
        }

        MethodModel main_method = null;
        for (var method : clazz.methods()) {
            var type = method.methodTypeSymbol();
            System.out.print("Method: " + flags(method.flags()) + type.returnType().displayName() + " " + method.methodName().stringValue() + "(");
            System.out.print(Arrays.stream(type.parameterArray()).map(ClassDesc::displayName).collect(Collectors.joining(", ")));
            System.out.println(")");
            if (!method.methodName().stringValue().equals("main"))
                continue;
            main_method = method;
            var codeAttrOpt = method.findAttribute(Attributes.code());
            if (codeAttrOpt.isEmpty()) {
                System.out.println("No code for method.");
                continue;
            }
            CodeAttribute attrib = codeAttrOpt.get();
//            boolean creates_object = false;
            // 1p03
            boolean creates_object = true;
            for (var el : attrib.elementList()) {
                if (el instanceof NewObjectInstruction newObj) {
                    if (newObj.className().name() != clazz.thisClass().name()) {
                        System.err.println("User isn't creating an object of the class he's playing with.");
                        System.err.println("I am going to assume the presence of this line means it will create the object");
                        System.err.println("Calling main function!");
                    }
                    creates_object = true;
                    break;
                }
            }
            if (!creates_object)
                main_method = null;
        }
        // Compute the binary name (e.g., "pkg.Foo") from internal name (e.g., "pkg/Foo")
        String internalName = String.valueOf(clazz.thisClass().name());
        String binaryName = internalName.replace('/', '.');

        var parent = new java.net.URLClassLoader(urls, ClassLoader.getSystemClassLoader());

        // Load the class bytes into an isolated loader to avoid linkage conflicts
        var loader = new BytesClassLoader(parent);

        try {
            Class<?> target = loader.define(binaryName, bytes);
            if (main_method == null) {
                System.out.println("No main method found, trying to construct a new instance of the class");
                for (var constructor : target.getDeclaredConstructors()){
                    if (constructor.getParameterCount() == 0){
                        System.out.println("Constructing new instance of " + binaryName);
                        wait_for_frame_close();
                        constructor.newInstance();
                        break;
                    }
                }
                return;
            }
            // Get main(String[]) and forward remaining CLI args (after the .class path)
            var m = target.getMethod("main", String[].class);

            System.out.println("Invoking " + binaryName + ".main");
            wait_for_frame_close();
            m.invoke(null, (Object) new String[0]);
        } catch (Throwable t) {
            System.err.println("Failed to invoke main: " + t);
            t.printStackTrace();
        }
    }

    static void wait_for_frame_close(){
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
                for (var frame : active_frames){
                    if (!frame.isVisible()){
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
