#include <KEWindow.h>
#include "KEVulkanRHI\VulkanRenderer.h"


int main() {
	setvbuf(stdout, NULL, _IONBF, 0);                    // Prevent printf buffering in QtCreator
	printf("WSI-Window\n");
	LOGW("Test Warnings\n");

	//Startup Window
	{
		KEWindowDesc l_desc = { "KEngine",2560,0,1920,1080 };
		KEWindow::StartUp(l_desc);
	}
	

	//Startup Renderer
	{
		KEVulkanRendererDescriptor l_renderer_desc = {};
		l_renderer_desc.enable_validation = true;
		KEVulkanRenderer::StartUp(l_renderer_desc, KEWindow::InstancePtr());
	}


	
	//Main Loop
	{
		auto& Window = KEWindow::Instance();
		while (Window.ProcessEvents()) {
			bool key_pressed = Window.GetKeyState(KEY_LeftShift);
			if (key_pressed) printf("LEFT SHIFT PRESSED\r");
		}
	}

	//Shut Down
	{
		KEWindow::ShutDown();
		KEVulkanRenderer::ShutDown();
	}

	return 0;
}