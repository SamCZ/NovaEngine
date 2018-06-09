#include "Engine/Blueprint/BlueprintModule.h"

namespace NovaEngine {
	BlueprintModule::BlueprintModule(std::string name) {
		_name = name;
	}

	std::string BlueprintModule::getName() const {
		return _name;
	}

	StreamLink* BlueprintModule::createInputLink(const std::string& name, const LinkVarType& type, bool reachable) {

		return nullptr;
	}

	StreamLink* BlueprintModule::createOutputLink(const std::string& name, const LinkVarType& type, StreamLink* link) {
		return nullptr;
	}

	void BlueprintModule::setInput(const std::string& name, StreamLink* link) {
	}

	StreamLink* BlueprintModule::getOutput(const std::string& name) {
		return nullptr;
	}
}