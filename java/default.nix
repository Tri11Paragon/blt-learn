{ pkgs ? (import <nixpkgs> { 
    config.allowUnfree = true;
    config.segger-jlink.acceptLicense = true; 
}), customPkgs ? (import <nixos-unstable> {
	config.allowUnfree = true;
    config.segger-jlink.acceptLicense = true;
}), ... }:
pkgs.mkShell
{
	buildInputs = with pkgs; [
		cmake 
		gcc
		clang
		emscripten
		ninja
		customPkgs.jetbrains.idea-ultimate
		#clion = import ~/my-nixpkgs/pkgs/applications/editors/jetbrains {};
		renderdoc
		valgrind
		customPkgs.openjdk24
		xorg.libX11 
		xorg.libXext 
		xorg.libXi 
		xorg.libXtst 
		xorg.libXt 
		xorg.libXrender 
		xorg.libXrandr 
		xorg.libXfixes 
		xorg.libXcursor 
		xorg.libXinerama 
		fontconfig 
		freetype 
		alsa-lib
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
	];
	LD_LIBRARY_PATH="/run/opengl-driver/lib:/run/opengl-driver-32/lib";
}
