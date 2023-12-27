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
	float NOISE_SEED       = 1.0;
	float NOISE_MAX_VALUE  = 1.0;
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
		if (ImGui::TreeNode("Noise Map")) {
			ImGui::DragFloat("Constract", &NOISE_SEED, 0.01, 0.0f, 3.0f);
			ImGui::DragFloat("Max Value", &NOISE_MAX_VALUE, 0.01, 0.0f, 3.0f);

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
		s.SetUniform1f("NOISE_SEED", NOISE_SEED);
		s.SetUniform1f("MAX_VALUE", NOISE_MAX_VALUE);



		s.SetUniform3f("OCEAN_COLOR", OCEAN_COLOR.x, OCEAN_COLOR.y, OCEAN_COLOR.z);
		s.SetUniform3f("PLAIN_COLOR", PLAIN_COLOR.x, PLAIN_COLOR.y, PLAIN_COLOR.z);
		s.SetUniform3f("MOUNTAIN_COLOR", MOUNTAIN_COLOR.x, MOUNTAIN_COLOR.y, MOUNTAIN_COLOR.z);
	}
};

class PlanetGenerator : public ShaderData, public Serialized {
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
	float NOISE_SEED     = 1.0f;
	float NOISE_MAX_VALUE = 1.0;
	int   APPLY_BORDER = 0;
	float BORDER_LEVEL = -0.4;
	float atmThickness = 0.095;
	float atmContrib   = 3.5;
	fVec3 atmCol       = fVec3(0.88, 0.88, 1.3);

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
	void Gui() {

		if (ImGui::Button("Save to file")) {
			std::string temp = SaveAs(WIN_STR_FILTER("*.planet"));
			if (temp.size() > 0) {
				std::fstream fs;
				fs.open(temp.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
				Serialize(&fs, 0);
				fs.close();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Load from file")) {
			std::string temp = GetFile(WIN_STR_FILTER("*.planet"));
			if (temp.size() > 0) {
				std::fstream fs;
				fs.open(temp.c_str(), std::ios::in | std::ios::binary);
				Deserialize(&fs,0);
				fs.close();
			}
		}

		if (ImGui::TreeNode("Noise Map")) {
			static bool tempFilter = 0;
			if (ImGui::Checkbox("Linear Texture Filtering", &tempFilter)) {
				TextureDataUpdate texData;
				texData.linear	  = tempFilter;
				texData.genMipMap = 0;
				Renderer::defaultRenderer->componentContainer.GetComponent<Camera>()->fbo.RenderedImage.UpdateTextureData(texData);
				Renderer::currentRenderer->componentContainer.GetComponent<Camera>()->fbo.RenderedImage.UpdateTextureData(texData);
			}

			ImGui::DragFloat("Constract", &NOISE_SEED, 0.01, 0.0f, 3.0f);
			ImGui::DragFloat("Max Value", &NOISE_MAX_VALUE, 0.01, 0.0f, 3.0f);

			ImGui::DragFloat("Level of details", &LOD, 0.01, 0.0f, 100.0f);
			ImGui::DragFloat("Ocean Level", &OCEAN_LEVEL, 0.01, -2.0f, 2.0f);
			ImGui::DragFloat("Ocean Edge", &OCEAN_EDGE, 0.01f, -2.0f, 2.0f);
			ImGui::DragFloat("Mountain Level", &MOUNTAIN_LEVEL, 0.01f, -2.0f, 2.0f);
			ImGui::DragFloat("Mountain Level2", &MOUNTAIN_LEVEL_2, 0.01f, -2.0f, 2.0f);

			bool a = APPLY_BORDER;

			if (ImGui::Checkbox("Apply Borders", &a)) {
				APPLY_BORDER = a;
			}

			if (APPLY_BORDER) {
				ImGui::DragFloat("Border Level", &BORDER_LEVEL, 0.01f, -2.0f, 2.0f);
			}
			

			ImGui::DragFloat3("Ocean Color", &OCEAN_COLOR.x, 0.01, 0.0, 1.0);
			ImGui::DragFloat3("Plain Color", &PLAIN_COLOR.x, 0.01, 0.0, 1.0);
			ImGui::DragFloat3("Mountain Color", &MOUNTAIN_COLOR.x, 0.01, 0.0, 1.0);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Planet")) {
			bool b = CHEAP_NOISE;
			ImGui::DragFloat("Rotation", &PLANET_ROTATION.x, 0.01);
			if (ImGui::Checkbox("CHEAP_NOISE", &b)) {
				CHEAP_NOISE = b;
			}
			
			ImGui::DragFloat("Maximum height", &MAX_HEIGHT, 0.001, -0.05, 0.05);
			ImGui::DragFloat("Atmosphere Thickness",		  &atmThickness, 0.001, 0.00001, 1.0);
			ImGui::DragFloat("Atmosphere Color Contribution", &atmContrib,   0.01, 1.0);
			ImGui::DragFloat3("Atmosphere Color", &atmCol.x,0.01,0.0,1.0);

			ImGui::DragFloat3("Light Position", &LIGHT_POSITION.x, 0.01 );
			
			ImGui::TreePop();
		}
	};

	void SetUniforms() {
		Shader& s = *shader;
		Camera& camC = *Scene::currentScene->GetGameObject("Cam")->GetComponent<Camera>();

		s.Use();

		s.SetUniform1f("NOISE_SEED", NOISE_SEED);
		s.SetUniform1f("MAX_VALUE", NOISE_MAX_VALUE);

		
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
		s.SetUniform3f("atmCol", atmCol.x, atmCol.y, atmCol.z);

		s.SetUniform3f("OCEAN_COLOR", OCEAN_COLOR.x, OCEAN_COLOR.y, OCEAN_COLOR.z);
		s.SetUniform3f("PLAIN_COLOR", PLAIN_COLOR.x, PLAIN_COLOR.y, PLAIN_COLOR.z);
		s.SetUniform3f("MOUNTAIN_COLOR", MOUNTAIN_COLOR.x, MOUNTAIN_COLOR.y, MOUNTAIN_COLOR.z);

		s.SetUniform3f("LIGHT_POSITION", LIGHT_POSITION.x, LIGHT_POSITION.y, LIGHT_POSITION.z);


		s.SetVector2("PLANET_ROTATION", PLANET_ROTATION.x, PLANET_ROTATION.y);
		s.SetUniform1i("CHEAP_NOISE", CHEAP_NOISE);
	};

	int Serialize(std::fstream* data, int offset) override {
		int buf = 0;
		WRITE_ON_BIN(data, &LOD, sizeof(LOD),buf);
		WRITE_ON_BIN(data, &OCEAN_LEVEL, sizeof(OCEAN_LEVEL), buf);
		WRITE_ON_BIN(data, &OCEAN_EDGE, sizeof(OCEAN_EDGE), buf);
		WRITE_ON_BIN(data, &MOUNTAIN_LEVEL, sizeof(MOUNTAIN_LEVEL), buf);
		WRITE_ON_BIN(data, &MOUNTAIN_LEVEL_2, sizeof(MOUNTAIN_LEVEL_2), buf);
		WRITE_ON_BIN(data, &MAX_HEIGHT, sizeof(MAX_HEIGHT), buf);
		WRITE_ON_BIN(data, &NOISE_SEED, sizeof(NOISE_SEED), buf);
		WRITE_ON_BIN(data, &NOISE_MAX_VALUE, sizeof(NOISE_MAX_VALUE), buf);
		WRITE_ON_BIN(data, &APPLY_BORDER, sizeof(APPLY_BORDER), buf);
		WRITE_ON_BIN(data, &BORDER_LEVEL, sizeof(BORDER_LEVEL), buf);
		WRITE_ON_BIN(data, &atmThickness, sizeof(atmThickness), buf);
		WRITE_ON_BIN(data, &atmContrib, sizeof(atmContrib), buf);
		WRITE_ON_BIN(data, &CHEAP_NOISE, sizeof(CHEAP_NOISE), buf);

		WRITE_ON_BIN(data, &OCEAN_COLOR.x, sizeof(OCEAN_COLOR.x), buf);
		WRITE_ON_BIN(data, &OCEAN_COLOR.y, sizeof(OCEAN_COLOR.y), buf);
		WRITE_ON_BIN(data, &OCEAN_COLOR.z, sizeof(OCEAN_COLOR.z), buf);

		WRITE_ON_BIN(data, &PLAIN_COLOR.x, sizeof(PLAIN_COLOR.x), buf);
		WRITE_ON_BIN(data, &PLAIN_COLOR.y, sizeof(PLAIN_COLOR.y), buf);
		WRITE_ON_BIN(data, &PLAIN_COLOR.z, sizeof(PLAIN_COLOR.z), buf);

		WRITE_ON_BIN(data, &MOUNTAIN_COLOR.x, sizeof(MOUNTAIN_COLOR.x), buf);
		WRITE_ON_BIN(data, &MOUNTAIN_COLOR.y, sizeof(MOUNTAIN_COLOR.y), buf);
		WRITE_ON_BIN(data, &MOUNTAIN_COLOR.z, sizeof(MOUNTAIN_COLOR.z), buf);

		WRITE_ON_BIN(data, &PLANET_ROTATION.x, sizeof(PLANET_ROTATION.x), buf);
		PLANET_ROTATION.y  = 0;

		WRITE_ON_BIN(data, &LIGHT_POSITION.x, sizeof(LIGHT_POSITION.x), buf);
		WRITE_ON_BIN(data, &LIGHT_POSITION.y, sizeof(LIGHT_POSITION.y), buf);
		WRITE_ON_BIN(data, &LIGHT_POSITION.z, sizeof(LIGHT_POSITION.z), buf);

		WRITE_ON_BIN(data, &atmCol.x, sizeof(atmCol.x), buf);
		WRITE_ON_BIN(data, &atmCol.y, sizeof(atmCol.y), buf);
		WRITE_ON_BIN(data, &atmCol.z, sizeof(atmCol.z), buf);

		return 0;
	}

	int Deserialize(std::fstream* data, int offset) override {
		int buf = 0;
		READ_FROM_BIN(data, &LOD, buf);
		READ_FROM_BIN(data, &OCEAN_LEVEL, buf);
		READ_FROM_BIN(data, &OCEAN_EDGE, buf);
		READ_FROM_BIN(data, &MOUNTAIN_LEVEL, buf);
		READ_FROM_BIN(data, &MOUNTAIN_LEVEL_2, buf);
		READ_FROM_BIN(data, &MAX_HEIGHT, buf);
		READ_FROM_BIN(data, &NOISE_SEED, buf);
		READ_FROM_BIN(data, &NOISE_MAX_VALUE, buf);

		READ_FROM_BIN(data, &APPLY_BORDER, buf);
		READ_FROM_BIN(data, &BORDER_LEVEL, buf);
		READ_FROM_BIN(data, &atmThickness, buf);
		READ_FROM_BIN(data, &atmContrib, buf);
		READ_FROM_BIN(data, &CHEAP_NOISE, buf);

		READ_FROM_BIN(data, &OCEAN_COLOR.x, buf);
		READ_FROM_BIN(data, &OCEAN_COLOR.y, buf);
		READ_FROM_BIN(data, &OCEAN_COLOR.z, buf);

		READ_FROM_BIN(data, &PLAIN_COLOR.x, buf);
		READ_FROM_BIN(data, &PLAIN_COLOR.y, buf);
		READ_FROM_BIN(data, &PLAIN_COLOR.z, buf);

		READ_FROM_BIN(data, &MOUNTAIN_COLOR.x, buf);
		READ_FROM_BIN(data, &MOUNTAIN_COLOR.y, buf);
		READ_FROM_BIN(data, &MOUNTAIN_COLOR.z, buf);

		READ_FROM_BIN(data, &PLANET_ROTATION.x, buf);

		READ_FROM_BIN(data, &LIGHT_POSITION.x, buf);
		READ_FROM_BIN(data, &LIGHT_POSITION.y, buf);
		READ_FROM_BIN(data, &LIGHT_POSITION.z, buf);

		READ_FROM_BIN(data, &atmCol.x, buf);
		READ_FROM_BIN(data, &atmCol.y, buf);
		READ_FROM_BIN(data, &atmCol.z, buf);

		return 0;
	}
};


void Render() {
	(*Renderer::currentRenderer)(false);					       //Update renderer camera
	(*Renderer::defaultRenderer)(Renderer::currentRenderer, true); //Redraw with stretching perhaps
};

void controller::Start() {
	Camera& camC = *Scene::currentScene->GetGameObject("Cam")->GetComponent<Camera>();
	resolutionTmp[0] = camC.size.x;
	resolutionTmp[1] = camC.size.y;

	static ShaderData	   uniformColor = ShaderData();
	static NoiseMap		   noiseMap     = NoiseMap();
	static PlanetGenerator pg			= PlanetGenerator();
	mapp[noiseMap.type]			   = &noiseMap;
	mapp[uniformColor.type]		   = &uniformColor;
	mapp[pg.type] = &pg;

	Renderer::currentRenderer->SetShader(mapp[currentShader]->shader->name);
	NWenginePushFunction(ON_MAIN_CALL_LOCATION::FrameIntermediate, &Render);
};

void controller::Update() {
	Camera&    camC = *Scene::currentScene->GetGameObject("Cam")->GetComponent<Camera>();
	Renderer&  rend = *Renderer::currentRenderer;
	//---------Cam UI--------------
	
	ImGui::Begin("Gui");
	ImGui::LabelText("##FPS00", (("FPS: " + std::to_string(NWTime::GetFPS())).c_str()));
	if (ImGui::TreeNode("Camera Parameters")) {
		ImGui::DragInt2("Shader Resolution Value", resolutionTmp);
		if (ImGui::Button("Apply Resolution Change")) {
			camC.ChangeOrtho(resolutionTmp[0], resolutionTmp[1]);
		}
		ImGui::DragFloat2("Camera Stretch Value", &Renderer::defaultRenderer->strechCoeff.x, 0.01, 0.0f, 100.0f);

		ImGui::TreePop();
	}


	//----------------------------
	if (ImGui::BeginCombo("Shader", mapp[currentShader]->name.c_str())) {
		for (auto iter = mapp.begin(); iter != mapp.end(); ++iter) {
			if (ImGui::Selectable(mapp[iter->second->type]->name.c_str())) {
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