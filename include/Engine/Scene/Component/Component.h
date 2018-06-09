#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <map>

#include "Engine/Material/Technique.h"

namespace NovaEngine {
	class Spatial;

	class UVar {
	private:
		MaterialParam* _dataParam;
		std::string _name;
		bool _isButton;
	public:
		inline ~UVar() {

		}

		inline UVar() : _dataParam(nullptr), _isButton(false){}

		inline UVar(const std::string& name) : _dataParam(nullptr), _name(name), _isButton(true) {

		}

		template<typename T>
		inline UVar(std::string name, T data) : _name(name), _isButton(false) {
			_dataParam = new MaterialParam();
			_dataParam->Name = name;
			_dataParam->Label = name;
			_dataParam->IsPreset = true;
			_dataParam->setData<T>(data);
		}

		inline virtual std::string getName() {
			return _name;
		}

		inline MaterialParam* getParam() {
			return _dataParam;
		}

		inline virtual bool isButton() {
			return _isButton;
		}
	};

	template<typename T>
	class TVar : public UVar {
	public:
		inline ~TVar() {

		}

		inline TVar(std::string name, T data) : UVar(name, data) {

		}

		inline T get() {
			return _dataParam->getData<T>();
		}
	};

	typedef std::map<std::string, UVar> VarStorage;

	class Component {
	protected:
		Spatial* _spatial;
		std::string _name;
		VarStorage _variables;

		inline void addVar(UVar& var) {
			_variables[var.getName()] = var;
		}
	public:
		inline ~Component() {
			for (auto& v : _variables) {
				delete v.second.getParam();
			}
		}

		inline Component(const std::string& name) : _name(name) {

		}

		inline void preInit(Spatial* spatial) {
			_spatial = spatial;
		}

		inline std::string getName() {
			return _name;
		}

		inline VarStorage& getVariables() {
			return _variables;
		}

		virtual void init() = 0;
		virtual void update() = 0;
	};
}

#endif // !COMPONENT_H