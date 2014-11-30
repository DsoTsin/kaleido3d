#pragma once

class GLInitializer {
public:
	// @see GLDevice
	static void InitAndCheck();
	static bool NVCommandListSupported();
};