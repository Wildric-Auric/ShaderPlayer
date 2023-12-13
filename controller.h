#pragma once
#include "NWengineCore.h"

enum class ShaderCombo {
	NONE,
	MAP,
	PLANET
};


class controller : Scriptable {
public:
	SCRIPT_CONSTR(controller)
	
	ShaderCombo currentShader = ShaderCombo::NONE;

	void Start()  override;
	void Update() override;
};

