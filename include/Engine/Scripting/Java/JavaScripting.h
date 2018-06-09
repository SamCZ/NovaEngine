#ifndef JAVA_SCRITPING_H
#define JAVA_SCRITPING_H

#include "Engine/Scripting/Scripting.h"

#include <iostream>
#include <Windows.h>
#include <jni.h>

typedef jint(JNICALL *pCreateJavaVM)(JavaVM **, void**, void *);
HINSTANCE hInstance = LoadLibrary(L"C:\\Program Files\\Java\\jdk1.8.0_151\\jre\\bin\\server\\jvm.dll");
pCreateJavaVM CreateJavaVM = (pCreateJavaVM)GetProcAddress(hInstance, "JNI_CreateJavaVM");
#define CLEAR(x) memset(&x, 0, sizeof(x))

JNIEnv* create_vm(JavaVM** jvm) {
	JNIEnv *env;
	JavaVMInitArgs vm_args;
	CLEAR(vm_args);

	JavaVMOption options[3];
	CLEAR(options);

	options[0].optionString = "-Djava.class.path=C:\\Program Files\\Java\\jdk1.8.0_151\\lib";
	options[0].extraInfo = 0;
	options[1].optionString = "-Xms1m"; // 1MB
	options[1].extraInfo = 0;
	options[2].optionString = "-Xmx1g"; // 1GB
	options[2].extraInfo = 0;

	vm_args.version = JNI_VERSION_1_8;
	vm_args.nOptions = 3;
	vm_args.options = options;
	vm_args.ignoreUnrecognized = JNI_TRUE;

	int flag = CreateJavaVM(jvm, (void**)&env, &vm_args);
	if (flag == JNI_ERR) {
		std::cout << "Error creating VM. Exiting...\n";
	}
	return env;
}

namespace NovaEngine {
	class JavaScripting : public Scripting {
	private:
		JNIEnv* _env;
		JavaVM* _jvm;

		inline void initSystem() override {
			_env = create_vm(&_jvm);
		}

	public:
		inline ~JavaScripting() {
			_jvm->DestroyJavaVM();
		}

		inline JavaScripting() {

		}

		inline void init(Spatial* scene) {

		}

		inline void update(Spatial* scene) {

		}
	};
}

#endif // !JAVA_SCRITPING_H