#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <string>
#include <vector>
#include <map>
#include <assimp/scene.h>
#include "glm/glm.hpp"

#define MAX_BONES 64

namespace NovaEngine {

	class Spatial;

	struct Animation {
		std::string name;
		double duration;
		double ticksPerSecond;
		// all of the bone transformations, this is modified every frame
		// assimp calls it a channel, its anims for a node aka bone
		std::vector <glm::mat4> boneTrans;
		std::map <std::string, glm::mat4> boneOffset;

		struct Channel {
			std::string name;
			glm::mat4 offset;
			std::vector <aiVectorKey> mPositionKeys;
			std::vector <aiQuatKey> mRotationKeys;
			std::vector <aiVectorKey> mScalingKeys;
		};
		std::vector <Channel> channels;

		struct BoneNode {
			std::string name;
			BoneNode* parent;
			std::vector <BoneNode> children;
			glm::mat4 nodeTransform;
			glm::mat4 boneTransform;
		};
		BoneNode root;

		void buildBoneTree(const aiScene* scene, aiNode* node, BoneNode* bNode, Spatial* m);

		//aiAnimation* data;
	};

	class Animator {
	public:
		std::vector<Animation> animations;
		unsigned int currentAnim;
		std::map <std::string, unsigned int> boneID;

	};
}

#endif // !ANIMATOR_H