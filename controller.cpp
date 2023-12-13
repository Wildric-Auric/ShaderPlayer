#include "controller.h"
#include "imgui.h"
#include "map"
#include "string"




class ShaderData {
public:
	ShaderCombo type;
	std::string name;
	Shader*	    shader;

	ShaderData() {
		type   = ShaderCombo::NONE;
		name   = "Uniform Color";
		shader = RessourcesLoader::LoadShader("Ressources\\Shaders\\Textured.shader");
	}

	void Use() {
		shader->Use();
	}

	virtual void Gui() {
		Camera& camC = *Scene::currentScene->GetGameObject("Cam")->GetComponent<Camera>();
		ImGui::DragFloat3("COLCHANGE", &camC.clearColor.x, 0.01f, 0.0f, 1.0f);
	};

	virtual void SetUniforms() {};
	virtual ~ShaderData() {}
};

std::map<ShaderCombo, ShaderData*>  mapp;
static int resolutionTmp[2] = { 0.0, 0.0 };

class NoiseMap : public ShaderData {
public:
	float LOD			   = 1.0;
	float OCEAN_LEVEL	   = 0.45f;
	float OCEAN_EDGE	   = 0.1;
	float MOUNTAIN_LEVEL   = 0.85;
	float MOUNTAIN_LEVEL_2 = 0.95;
	fVec3 OCEAN_COLOR	   = fVec3(0.0, 0.0, 1.0);
	fVec3 PLAIN_COLOR	   = fVec3(0.0, 0.5, 0.0);
	fVec3 MOUNTAIN_COLOR   = fVec3(0.8, 0.8, 0.8);

	int APPLY_BORDER       = 1;
	float BORDER_LEVEL     = -0.4;
	NoiseMap() {
		name   = "Noise Map";
		type   = ShaderCombo::MAP;
		shader = RessourcesLoader::LoadShader("Shaders\\NoiseMap.shader");
	}

	void Gui() override {
		if (ImGui::TreeNode("mapgui")) {
			ImGui::DragFloat("Level of details", &LOD, 0.01, 0.0f, 100.0f);
			ImGui::DragFloat("Ocean Level", &OCEAN_LEVEL, 0.01, -2.0f, 2.0f);
			ImGui::DragFloat("Ocean Edge", &OCEAN_EDGE, 0.01f, -2.0f, 2.0f);
			ImGui::DragFloat("Mountain Level", &MOUNTAIN_LEVEL, 0.01f, -2.0f, 2.0f);
			ImGui::DragFloat("Mountain Level2", &MOUNTAIN_LEVEL_2, 0.01f, -2.0f, 2.0f);
			bool a = APPLY_BORDER;
			if (ImGui::Checkbox("APPLY_BORDER", &a)) 
				APPLY_BORDER = a;

			if (APPLY_BORDER)
				ImGui::DragFloat("Border Level", &BORDER_LEVEL, 0.01f, -2.0f, 2.0f);
			ImGui::DragFloat3("Ocean Color", &OCEAN_COLOR.x, 0.01, 0.0, 1.0);
			ImGui::DragFloat3("Plain Color", &PLAIN_COLOR.x, 0.01, 0.0, 1.0);
			ImGui::DragFloat3("Mountain Color", &MOUNTAIN_COLOR.x, 0.01, 0.0, 1.0);
			ImGui::TreePop();
		}
	}

	void SetUniforms() override {
		Shader& s = *shader;
	
		s.Use();
		s.SetUniform1f("OCEAN_LEVEL", OCEAN_LEVEL);
		s.SetUniform1f("OCEAN_EDGE", OCEAN_EDGE);
		s.SetUniform1f("MOUNTAIN_LEVEL", MOUNTAIN_LEVEL);
		s.SetUniform1f("MOUNTAIN_LEVEL_2", MOUNTAIN_LEVEL_2);
		s.SetUniform1f("LOD", LOD);
		s.SetUniform1i("APPLY_BORDER", APPLY_BORDER);
		s.SetUniform1f("BORDER_LEVEL", BORDER_LEVEL);

		s.SetUniform3f("OCEAN_COLOR", OCEAN_COLOR.x, OCEAN_COLOR.y, OCEAN_COLOR.z);
		s.SetUniform3f("PLAIN_COLOR", PLAIN_COLOR.x, PLAIN_COLOR.y, PLAIN_COLOR.z);
		s.SetUniform3f("MOUNTAIN_COLOR", MOUNTAIN_COLOR.x, MOUNTAIN_COLOR.y, MOUNTAIN_COLOR.z);
	}
};

static bool shouldDraw = 1;
class PlanetGenerator : public ShaderData {
public:


	//------------Map values---------------
	float LOD = 1.0;
	float OCEAN_LEVEL = 0.45f;
	float OCEAN_EDGE = 0.1;
	float MOUNTAIN_LEVEL = 0.85;
	float MOUNTAIN_LEVEL_2 = 0.95;
	fVec3 OCEAN_COLOR = fVec3(0.0, 0.0, 1.0);
	fVec3 PLAIN_COLOR = fVec3(0.0, 0.5, 0.0);
	fVec3 MOUNTAIN_COLOR = fVec3(0.8, 0.8, 0.8);
	float MAX_HEIGHT     = 0.001;
	int   APPLY_BORDER = 1;
	float BORDER_LEVEL = -0.4;
	float atmThickness = 0.095;
	float atmContrib   = 3.5;

	int   CHEAP_NOISE = 1;
	fVec2 PLANET_ROTATION = fVec2(0.0f, 0.0f);
	fVec3 LIGHT_POSITION  = fVec3(1.0, 0.0, 0.0);

	PlanetGenerator() {
		type = ShaderCombo::PLANET;
		name = "Planet Generator";
		shader = RessourcesLoader::LoadShader("Shaders\\PlanetGenerator.shader");
	}

	void Use() {
		shader->Use();
	}
#define G(line) shouldDraw |= line;
	void Gui() {
		shouldDraw = 0;

		if (ImGui::TreeNode("Noise Map")) {
			static bool tempFilter = 0;
			if (ImGui::Checkbox("Linear Texture Filtering", &tempFilter)) {
				TextureDataUpdate texData;
				texData.linear	  = tempFilter;
				texData.genMipMap = 0;
				Renderer::defaultRenderer->componentContainer.GetComponent<Camera>()->fbo.RenderedImage.UpdateTextureData(texData);
				Renderer::currentRenderer->componentContainer.GetComponent<Camera>()->fbo.RenderedImage.UpdateTextureData(texData);
				shouldDraw = 1;
			}

			G(ImGui::DragFloat("Level of details", &LOD, 0.01, 0.0f, 100.0f));
			G(ImGui::DragFloat("Ocean Level", &OCEAN_LEVEL, 0.01, -2.0f, 2.0f));
			G(ImGui::DragFloat("Ocean Edge", &OCEAN_EDGE, 0.01f, -2.0f, 2.0f));
			G(ImGui::DragFloat("Mountain Level", &MOUNTAIN_LEVEL, 0.01f, -2.0f, 2.0f));
			G(ImGui::DragFloat("Mountain Level2", &MOUNTAIN_LEVEL_2, 0.01f, -2.0f, 2.0f));

			bool a = APPLY_BORDER;

			if (ImGui::Checkbox("APPLY_BORDER", &a)) {
				APPLY_BORDER = a;
				shouldDraw   = 1;
			}

			if (APPLY_BORDER) {
				G(ImGui::DragFloat("Border Level", &BORDER_LEVEL, 0.01f, -2.0f, 2.0f));
			}
			

			G(ImGui::DragFloat3("Ocean Color", &OCEAN_COLOR.x, 0.01, 0.0, 1.0));
			G(ImGui::DragFloat3("Plain Color", &PLAIN_COLOR.x, 0.01, 0.0, 1.0));
			G(ImGui::DragFloat3("Mountain Color", &MOUNTAIN_COLOR.x, 0.01, 0.0, 1.0));
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Planet")) {
			bool b = CHEAP_NOISE;
			G(ImGui::DragFloat2("PLANET_ROTATION", &PLANET_ROTATION.x, 0.01));
			if (ImGui::Checkbox("CHEAP_NOISE", &b)) {
				CHEAP_NOISE = b;
				shouldDraw  = 1;
			}
			
			G(ImGui::DragFloat("Maximum height", &MAX_HEIGHT, 0.001, -0.05, 0.05));
			G(ImGui::DragFloat("Atmosphere Thickness",		  &atmThickness, 0.001, 0.00001, 1.0));
			G(ImGui::DragFloat("Atmosphere Color Contribution", &atmContrib,   0.01, 1.0));
			G(ImGui::DragFloat3("Light Position", &LIGHT_POSITION.x, 0.01 ));
			
			ImGui::TreePop();
		}
	};

	void SetUniforms() {
		Shader& s = *shader;
		Camera& camC = *Scene::currentScene->GetGameObject("Cam")->GetComponent<Camera>();

		s.Use();
		
		s.SetVector2("uRes", camC.size.x, camC.size.y);

		s.SetUniform1f("OCEAN_LEVEL", OCEAN_LEVEL);
		s.SetUniform1f("OCEAN_EDGE", OCEAN_EDGE);
		s.SetUniform1f("MOUNTAIN_LEVEL", MOUNTAIN_LEVEL);
		s.SetUniform1f("MOUNTAIN_LEVEL_2", MOUNTAIN_LEVEL_2);
		s.SetUniform1f("LOD", LOD);
		s.SetUniform1i("APPLY_BORDER", APPLY_BORDER); 
		s.SetUniform1f("BORDER_LEVEL", BORDER_LEVEL);
		s.SetUniform1f("MAX_HEIGHT", MAX_HEIGHT);
		s.SetUniform1f("atmThickness", atmThickness);
		s.SetUniform1f("atmContrib", atmContrib);

		s.SetUniform3f("OCEAN_COLOR", OCEAN_COLOR.x, OCEAN_COLOR.y, OCEAN_COLOR.z);
		s.SetUniform3f("PLAIN_COLOR", PLAIN_COLOR.x, PLAIN_COLOR.y, PLAIN_COLOR.z);
		s.SetUniform3f("MOUNTAIN_COLOR", MOUNTAIN_COLOR.x, MOUNTAIN_COLOR.y, MOUNTAIN_COLOR.z);

		s.SetUniform3f("LIGHT_POSITION", LIGHT_POSITION.x, LIGHT_POSITION.y, LIGHT_POSITION.z);


		s.SetVector2("PLANET_ROTATION", PLANET_ROTATION.x, PLANET_ROTATION.y);
		s.SetUniform1i("CHEAP_NOISE", CHEAP_NOISE);
	};
};


void Render() {
	(*Renderer::currentRenderer)(false);					       //Update renderer camera
	(*Renderer::defaultRenderer)(Renderer::currentRenderer, true); //Redraw with stretching perhaps
};

void controller::Start() {
	Camera& camC = *Scene::currentScene->GetGameObject("Cam")->GetComponent<Camera>();
	resolutionTmp[0] = camC.size.x;
	resolutionTmp[1] = camC.size.y;

	static ShaderData uniformColor = ShaderData();
	static NoiseMap   noiseMap     = NoiseMap();
	static PlanetGenerator pg      = PlanetGenerator();
	mapp[noiseMap.type]			   = &noiseMap;
	mapp[uniformColor.type]		   = &uniformColor;
	mapp[pg.type] = &pg;

	NWenginePushFunction(ON_MAIN_CALL_LOCATION::FrameIntermediate, &Render);
};

void controller::Update() {
	Camera&    camC = *Scene::currentScene->GetGameObject("Cam")->GetComponent<Camera>();
	Renderer&  rend = *Renderer::currentRenderer;
	//---------Cam UI--------------
	
	ImGui::Begin("Gui");
	ImGui::LabelText("FPS00", (("FPS: " + std::to_string(NWTime::GetFPS())).c_str()));
	if (ImGui::TreeNode("Camera Parameters")) {
		ImGui::DragInt2("Shader Resolution Value", resolutionTmp);
		if (ImGui::Button("Apply Resolution Change")) {
			camC.ChangeOrtho(resolutionTmp[0], resolutionTmp[1]);
			shouldDraw = 1;
		}
		ImGui::DragFloat2("Camera Stretch Value", &Renderer::defaultRenderer->strechCoeff.x, 0.01, 0.0f, 100.0f);

		ImGui::TreePop();
	}


	//----------------------------
	if (ImGui::BeginCombo("Shader", mapp[currentShader]->name.c_str())) {
		for (auto iter = mapp.begin(); iter != mapp.end(); ++iter) {
			if (ImGui::Selectable(mapp[iter->second->type]->name.c_str())) {
				shouldDraw	  = 1;
				currentShader = iter->second->type;
				rend.SetShader(mapp[iter->second->type]->shader->name);
				break;
			}
		}
		ImGui::EndCombo();
	}

	mapp[currentShader]->Gui();
	mapp[currentShader]->SetUniforms();

	ImGui::End();
}




Scriptable* controller::GetScript(GameObject* g) {
	return new controller(g);
};