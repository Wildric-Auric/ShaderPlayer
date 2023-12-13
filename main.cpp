#include "controller.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

void InitImGui() {
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)Context::window, true);
	ImGui_ImplOpenGL3_Init();
};

void BeginImGuiFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void DrawImGui() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


static Renderer renderer;
void InitScene() {

	renderer = Renderer();
	renderer.Use();

	Scene::currentScene	   = new Scene("");
	Scene* s			   = Scene::currentScene;
	GameObject& controller = s->AddObject(); s->Rename("Controller", &controller);
	GameObject& camera     = s->AddObject(); s->Rename("Cam", &camera);

	controller.AddComponent<Script>()->script = controller::GetScript(&controller);
	
	Camera*		camC       = camera.AddComponent<Camera>();
	
	camC->clearColor       = fVec3(0.0, 1.0, 0.0);
	camC->Use();


	Scene::currentScene->Start();
}




int main() {	
	
	NWenginePushFunction(ON_MAIN_CALL_LOCATION::InitEnd, InitImGui);
	NWenginePushFunction(ON_MAIN_CALL_LOCATION::InitEnd, InitScene);
	NWengineInit();

	//----------- Init, update, clean -------------
	NWenginePushFunction(ON_MAIN_CALL_LOCATION::FrameBegin, BeginImGuiFrame);
	NWenginePushFunction(ON_MAIN_CALL_LOCATION::FrameIntermediate, DrawImGui); //Order is important when using same loc again

	NWengineLoop();
	NWengineShutdown();
	return 0;
}

