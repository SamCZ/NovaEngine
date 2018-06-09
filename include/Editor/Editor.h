#ifndef NOVA_EDITOR_H
#define NOVA_EDITOR_H

#include "Engine/NovaEngine.h"
#include "Engine/Post/PostFilter.h"
#include "Engine/Control/GizmoControl.h"
#include "Engine/Render/PaintableTexture.h"
#include "Editor/Project.h"
#include "Editor/FileTree.h"
#include "Engine/Blueprint/Blueprint.h"

#include "Engine/Terrain/TerrainNode.h"
#include "Engine/Render/GPU/NormalMapGen.h"
#include "Engine/Render/ScreenSpaceSky.h"

using namespace NovaEngine;

class IGizmo;

class Editor : public Engine {
private:
	Project* _activeProject;
	FileTree* _assetTree;
	FileTree* _selectedFileTree;
	int _fileTreeNodeIndex;
	std::vector<FileTreeQueueItem> _assetDeleteQueue;
	FileType _inspectorMode;
	FileTreeQueueItem* _selectedTexture;

	std::vector<File> _shaders;
	std::string _shaderCombo;
	std::map<int, File*> _shadersArrayMap;

	Texture* _folderIcon;
	Texture* _unloadedIcon;
	Texture* _materialIcon;
	Texture* _modelIcon;
	Texture* _sceneIcon;
	Texture* _cameraIcon;

	Texture* _lightIcon;
	Texture* _lightBulbIcon;

	Camera* _editorCamera;

	bool _uiFocused;

	GizmoControl* _gizmoControl;
	bool _playMode;

	Material* _firstMaterial;

	Spatial* _selectedSpatial;
	Material* _selectedMaterial;
	Texture* _draggedTexture;
	//Texture* _draggingTexture;
	Material* _draggedMaterial;
	Spatial* _draggingSpatial;

	ScreenSpaceSky* _sky;
	SpriteBatch* _cloudBatch;
	float _time;
	int _grassDistance;
	float _grassDensity;

	/*std::vector<Material*> _materials;
	std::vector<Spatial*> _models;*/
	bool _isShiftDown;

	PostFilters* _post;

	int _fileWindowMode;

	bool _middle;
	bool _paint;
	bool _materialWindowOpened;
	bool _inspectorSceneMode;
	bool _skyRenderingEnabled;
	bool _updateShadowMaps = false;

	inline int getSizePercent(bool w, int per) {
		int size = _viewPort->getWidth();
		if(!w) size = _viewPort->getHeight();
		return ((float)size / 100.0f) * (float) per;
	}

	inline ImVec2 getScaledTexByHeight(Texture* tex, int height) {
		float aspect = (float)tex->getWidth() / (float)tex->getHeight();
		return ImVec2((int)(aspect * (float)height), height);
	}

	bool drawParamData(MaterialParam* param, TechniqueDef* technique);
	void drawMaterialWindow();
	void drawFileWindow();

	void drawLightDebug(Spatial* spatial, IdProvider& ids);
	void drawInspector(Spatial* obj, Material*& _selectedMaterial, Material* draggedMaterial, FileTreeQueueItem* item, const FileType& mode, bool &_materialWindowOpened, Camera* camera);
	void drawSceneInspector(Spatial* spatial, Spatial*& selected, Material*& selectedMaterial, IdProvider& provider, GizmoControl* gControl, FileType& inspectorMode);

	void initWater();
	void renderWater();

	Camera* getActiveCamera();
	void setCameraPlayMode(bool play);

	float waterY = 0;
	Camera* _waterCamera;
	FrameBuffer* reflectionFb;
	FrameBuffer* refractionFb;
	Material* waterMat;
	float moveFactor = 0;

	float _timeMult = 1.0;
public:
	~Editor();
	void init() override;
	void update() override;
	void render() override;

	void renderUI() override;
	void renderImGui() override;

	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseEvent(ButtonEvent e);
	void onKeyEvent(KeyEvent e);
};

#endif // !NOVA_EDITOR_H