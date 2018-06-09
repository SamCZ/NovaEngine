#ifndef BLUEPRINT_MODULE_H
#define BLUEPRINT_MODULE_H

#include <string>
#include <map>

namespace NovaEngine {
	enum class LinkType {
		Input, Output
	};

	enum class LinkVarType {
		Int,
		Float,
		Vector2,
		Vector3,
		Vector4,
		Matrix3,
		Matrix4,
		Texture,
		Bool
	};

	class StreamLink {
	private:

	public:
		inline StreamLink(int i) {}

		int getInt();
	};



	enum class BasicOperation {
		Add,
		Multiply
	};

	class BasicOperationLink : public StreamLink {
	private:

	public:
		BasicOperationLink(const BasicOperation& operation, StreamLink* input1, StreamLink* input2);
	};

	class BlueprintLink {
	public:
		std::string _name;
		LinkVarType _varType;
		bool _reachable;
		LinkType _type;
		StreamLink* _link = nullptr;

		inline BlueprintLink(const std::string& name, const LinkVarType& varType, bool reachable, const LinkType& type) {
			_name = name;
			_varType = varType;
			_reachable = reachable;
			_type = type;
		}
	};

	class BlueprintModule {
	private:
		std::string _name;
		std::map<std::string, BlueprintLink*> _inputLinks;
		std::map<std::string, BlueprintLink*> _outputLinks;
	public:
		BlueprintModule(std::string name);
		std::string getName() const;

		StreamLink* createInputLink(const std::string& name, const LinkVarType& type, bool reachable = true);
		StreamLink* createOutputLink(const std::string& name, const LinkVarType& type, StreamLink* link);

		void setInput(const std::string& name, StreamLink* link);
		StreamLink* getOutput(const std::string& name);
	};
}

#endif // !BLUEPRINT_MODULE_H