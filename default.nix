{ pkgs ? (import <nixpkgs> { 
    config.allowUnfree = true;
    config.segger-jlink.acceptLicense = true; 
}), customPkgs ? (import /home/brett/my-nixpkgs {
	config.allowUnfree = true;
    config.segger-jlink.acceptLicense = true;
}), unstable ? (import <nixos-unstable> {
	config.allowUnfree = true;
}), ... }:
pkgs.mkShell
{
	packages = [
		unstable.python312Packages.torchWithRocm
		(pkgs.python312.withPackages (python-pkgs: [
          python-pkgs.gymnasium
          python-pkgs.pybox2d
          python-pkgs.numpy
          python-pkgs.matplotlib
          python-pkgs.seaborn
          python-pkgs.pygame
          python-pkgs.tqdm
          python-pkgs.opencv4
    #     python-pkgs.torchWithRocm
        ]))
	];
	buildInputs = with pkgs; [
		cmake 
		gcc
		clang
		emscripten
		ninja
		unstable.jetbrains.clion
		#clion = import ~/my-nixpkgs/pkgs/applications/editors/jetbrains {};
		renderdoc
		valgrind
#		javaPackages.compiler.openjdk25
	];
	propagatedBuildInputs = with pkgs; [
		xorg.libX11 
		xorg.libX11.dev
		xorg.libXcursor
		xorg.libXcursor.dev
		xorg.libXext 
		xorg.libXext.dev
		xorg.libXinerama
		xorg.libXinerama.dev 
		xorg.libXrandr 
		xorg.libXrandr.dev
		xorg.libXrender
		xorg.libXrender.dev
		xorg.libxcb
		xorg.libxcb.dev
		xorg.libXi
		xorg.libXi.dev
		harfbuzz
		harfbuzz.dev
		zlib
		zlib.dev
		bzip2
		bzip2.dev
		pngpp
		brotli
		brotli.dev
		pulseaudio.dev
		git
		libGL
		libGL.dev
		glfw
		openmpi
	];
	LD_LIBRARY_PATH="/run/opengl-driver/lib:/run/opengl-driver-32/lib";
}
