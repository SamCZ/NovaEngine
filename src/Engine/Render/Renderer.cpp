#include "Engine/Render/Renderer.h"
#include "Engine/Utils/Buffers/FloatBuffer.h"
#include "Engine/Utils/Buffers/IntBuffer.h"
#include "Engine/Render/RenderManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#define USE_PBR

#ifdef USE_PBR
#include "Engine/Loaders/AssetManager.h"
#include "Engine/Material/Material.h"
#include "Engine/Render/Mesh/Shape.h"
#endif // USE_PBR

namespace NovaEngine {
	Renderer::Renderer() : _activeRenderState() {
		_activeRenderState.blendMode = BlendMode::Color;
		_activeRenderState.depthTest = true;
		_activeRenderState.faceCull = FaceCull::Back;
		_activeRenderState.lineWidth = 1.0f;
		_activeRenderState.wireframe = false;

		_firstStateInit = true;

		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glLineWidth(1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glGenVertexArrays(1, &_vertexArrayId);

		glGetIntegerv(GL_DRAW_BUFFER, &_initialDrawBuf);
		glGetIntegerv(GL_READ_BUFFER, &_initialReadBuf);
		_activeShader = nullptr;

		_lastMesh = nullptr;
	}

	Renderer::~Renderer() {
		glDeleteVertexArrays(1, &_vertexArrayId);
	}

	void Renderer::clearScreen(bool color, bool depth, bool stencil) {
		int flags = 0;
		if(color) flags |= GL_COLOR_BUFFER_BIT;
		if(depth) flags |= GL_DEPTH_BUFFER_BIT;
		if(stencil) flags |= GL_STENCIL_BUFFER_BIT;
		if(flags != 0) glClear(flags);
	}

	void Renderer::clearColor(float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
	}

	void Renderer::clearColor(ColorRGBA color) {
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void Renderer::setShader(Shader* shader) {
		if (_activeShader != shader | _invalidateShaderState) {
			_invalidateShaderState = false;
			_activeShader = shader;
			if (shader != nullptr) {
				if (!shader->isCompiled()) {
					shader->compile();
				}
				glUseProgram(shader->getId());
			} else {
				glUseProgram(0);
			}
		}
	}

	void Renderer::applyRenderState(RenderState& state) {
		if (_activeRenderState.depthTest != state.depthTest || _firstStateInit) {
			if (state.depthTest) {
				glEnable(GL_DEPTH_TEST);
			} else {
				glDisable(GL_DEPTH_TEST);
			}
			_activeRenderState.depthTest = state.depthTest;
		}

		if (_activeRenderState.blendMode != state.blendMode || _firstStateInit) {
			if (state.blendMode == BlendMode::Off) {
				glDisable(GL_BLEND);
				//glDisable(GL_ALPHA_TEST);
			} else {
				glEnable(GL_BLEND);
				if (state.blendMode == BlendMode::Additive) {
					glBlendFunc(GL_ONE, GL_ONE);
				} else if (state.blendMode == BlendMode::Alpha) {
					glEnable(GL_ALPHA_TEST);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				} else if (state.blendMode == BlendMode::AlphaAdditive) {
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				} else if (state.blendMode == BlendMode::Color || state.blendMode == BlendMode::Screen) {
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
				} else if (state.blendMode == BlendMode::PremultAlpha) {
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				}
			}
			_activeRenderState.blendMode = state.blendMode;
		}

		if (_activeRenderState.faceCull != state.faceCull || _firstStateInit) {
			if (state.faceCull == FaceCull::Off) {
				glDisable(GL_CULL_FACE);
			} else {
				glEnable(GL_CULL_FACE);
				if (state.faceCull == FaceCull::Back) {
					glCullFace(GL_BACK);
				} else if (state.faceCull == FaceCull::Front) {
					glCullFace(GL_FRONT);
				} else if (state.faceCull == FaceCull::FrontAndBack) {
					glCullFace(GL_FRONT_AND_BACK);
				}
			}
			_activeRenderState.faceCull = state.faceCull;
		}

		if (_activeRenderState.lineWidth != state.lineWidth || _firstStateInit) {
			glLineWidth(state.lineWidth);
			_activeRenderState.lineWidth = state.lineWidth;
		}

		if (_activeRenderState.wireframe != state.wireframe || _firstStateInit) {
			if (state.wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			} else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			_activeRenderState.wireframe = state.wireframe;
		}
		_firstStateInit = true;
	}

	void Renderer::finish() {

	}

	#pragma region "Set texture"

	void Renderer::setTexture(Texture* texture, int unit) {
		if (texture == nullptr) {
			glBindTexture(GL_TEXTURE_2D, 0);
			return;
		}

		int dataIndex = 0;
		if (Texture2DAnimated* anim = dynamic_cast<Texture2DAnimated*>(texture)) {
			if (anim) {
				if (!anim->isRunning()) {
					anim->start();
				}
				int prevFrame = anim->getCurrentFrame();
				anim->update();
				dataIndex = anim->getCurrentFrame();
				if (prevFrame != dataIndex) {
					texture->setNeedsUpdate(true);
				}
			}
		}
		
		GLenum target = GL_TEXTURE_2D;
		if (texture->getType() == TextureType::CubeMap) {
			target = GL_TEXTURE_CUBE_MAP;
		}

		GLuint id = texture->getTextureId();
		if (texture->isNeedsUpdate()) {
			if (id == -1) {
				glGenTextures(1, &id);
				texture->setTextureId(id);
			}

			ImageFormat format = texture->getFormat();
			if (format != ImageFormat::Custom) {
				if (texture->getType() == TextureType::CubeMap) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_CUBE_MAP, id);
					for (int i = 0; i < texture->getDataCount(); i++) {
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, getGLInternalFormat(format), texture->getWidth(), texture->getHeight(), 0, getGLFormat(format), getGLDataType(format), texture->getData(i));
					}
				}
				else {
					glBindTexture(GL_TEXTURE_2D, id);
					if (texture->isTherePixels()) {
						glTexImage2D(GL_TEXTURE_2D, 0, getGLInternalFormat(format), texture->getWidth(), texture->getHeight(), 0, getGLFormat(format), getGLDataType(format), texture->getData(dataIndex));
						texture->freeData();
					} else {
						glTexImage2D(GL_TEXTURE_2D, 0, getGLInternalFormat(format), texture->getWidth(), texture->getHeight(), 0, getGLFormat(format), getGLDataType(format), (void*)NULL);
					}
				}
			}
			texture->setNeedsUpdate(false);
		}

		if (texture->isNeedsGenerateMipmaps()) {
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			texture->setGenerateMipmaps(false);
		}

		glEnable(target);
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(target, id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		if (texture->isNeedsUpdateParams()) {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, (GLuint)texture->minificationFilter);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, (GLuint)texture->magnificationFilter);

			if (texture->wrapEnabled) {
				glTexParameteri(target, GL_TEXTURE_WRAP_S, (GLuint)texture->getWrap(WrapAxis::S));
				glTexParameteri(target, GL_TEXTURE_WRAP_T, (GLuint)texture->getWrap(WrapAxis::T));
				glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			}
			texture->setNeedsUpdateParams(false);
		}
	}

	void Renderer::clearActiveTextures() {
		for (int i = 0; i < 8; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
	}

	#pragma endregion

	void Renderer::setFrameBuffer(FrameBuffer* fb) {
		if (fb == nullptr) {
			bindFrameBuffer(nullptr);
		} else {
			if (fb->isNeedsUpdate()) {
				updateFrameBuffer(fb);
				setReadDrawBuffers(fb);
			} else {
				bindFrameBuffer(fb);
				setReadDrawBuffers(fb);
			}

			glViewport(0, 0, fb->getWidth(), fb->getHeight());
		}
	}

	void Renderer::bindFrameBuffer(FrameBuffer* fb) {
		if (fb == nullptr) {
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		} else {
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb->getId());
		}
	}

	void Renderer::updateFrameBuffer(FrameBuffer* fb) {
		if (fb->getNumColorBuffers() == 0 && fb->getDepthBuffer() == nullptr) {
			std::cerr << "The framebuffer doesn't have any color/depth buffers" << std::endl;
			return;
		}

		GLuint id = fb->getId();
		if (id == -1) {
			glGenFramebuffersEXT(1, &id);
			fb->setId(id);
		}

		bindFrameBuffer(fb);

		RenderBuffer* depthBuffer = fb->getDepthBuffer();
		if (depthBuffer != nullptr) {
			updateFrameBufferAttachment(fb, depthBuffer);
		}

		for (int i = 0; i < fb->getNumColorBuffers(); i++) {
			RenderBuffer* rb = fb->getColorBuffer(i);
			updateFrameBufferAttachment(fb, rb);
		}

		setReadDrawBuffers(fb);
		checkFrameBufferError();

		fb->markClean();
	}

	void Renderer::updateFrameBufferAttachment(FrameBuffer* fb, RenderBuffer* rb) {
		bool needAttach;
		if (rb->tex == nullptr) {
			needAttach = rb->id == -1;
			updateRenderBuffer(fb, rb);
		} else {
			needAttach = false;
			updateRenderTexture(fb, rb);
		}

		if (needAttach) {
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
										 convertAttachmentSlot(rb->slot),
										 GL_RENDERBUFFER_EXT,
										 rb->id);
		}
	}

	void Renderer::updateRenderBuffer(FrameBuffer* fb, RenderBuffer* rb) {
		GLuint id = rb->id;
		if (id == -1) {
			glGenRenderbuffersEXT(1, &id);
			rb->id = id;
		}
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, id);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, getGLInternalFormat(rb->format), fb->getWidth(), fb->getHeight());
	}

	void Renderer::updateRenderTexture(FrameBuffer* fb, RenderBuffer* rb) {
		setTexture(rb->tex, 0);
		if (rb->layer < 0) {
			GLenum target = GL_TEXTURE_2D;
			if (rb->tex->getType() == TextureType::CubeMap) {
				target = GL_TEXTURE_CUBE_MAP;
			}
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, convertAttachmentSlot(rb->slot), target, rb->tex->getTextureId(), 0);
		} else {
			glFramebufferTextureLayer(GL_FRAMEBUFFER_EXT, convertAttachmentSlot(rb->slot), rb->tex->getTextureId(), 0, rb->layer);
		}
	}

	void Renderer::setReadDrawBuffers(FrameBuffer* fb) {
		if (fb == nullptr) {
			glDrawBuffer(_initialDrawBuf);
			glReadBuffer(_initialReadBuf);
		} else {
			if (fb->getNumColorBuffers() == 0) {
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			} else {
				for (int i = 0; i < fb->getNumColorBuffers(); i++) {
					_attachments[i] = GL_COLOR_ATTACHMENT0 + i;
				}
				glDrawBuffers(fb->getNumColorBuffers(), _attachments);
			}
		}
	}

	int Renderer::convertAttachmentSlot(int attachmentSlot) {
		// can also add support for stencil here
		if (attachmentSlot == SLOT_DEPTH) {
			return GL_DEPTH_ATTACHMENT_EXT;
		} else if (attachmentSlot == SLOT_DEPTH_STENCIL) {
			// NOTE: Using depth stencil format requires GL3, this is already
			// checked via render caps.
			return GL_DEPTH_STENCIL_ATTACHMENT;
		} else if (attachmentSlot < 0 || attachmentSlot >= 16) {
			throw new std::exception(("Invalid FBO attachment slot: " + std::to_string(attachmentSlot)).c_str());
		}

		return GL_COLOR_ATTACHMENT0_EXT + attachmentSlot;
	}

	void Renderer::checkFrameBufferError() {
		GLuint status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		switch (status) {
			case GL_FRAMEBUFFER_COMPLETE_EXT:
			break;
			case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			//Choose different formats
			throw new std::exception("Framebuffer object format is unsupported by the video hardware.");
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			throw new std::exception("Framebuffer has erronous attachment.");
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			throw new std::exception("Framebuffer doesn't have any renderbuffers attached.");
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			throw new std::exception("Framebuffer attachments must have same dimensions.");
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			throw new std::exception("Framebuffer attachments must have same formats.");
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			throw new std::exception("Incomplete draw buffer.");
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			throw new std::exception("Incomplete read buffer.");
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
			throw new std::exception("Incomplete multisample buffer.");
			default:
			//Programming error; will fail on all hardware
			throw new std::exception("Some video driver error or programming error occured. Framebuffer object status is invalid.");
		}
	}

	#pragma region "Render Mesh"
	void Renderer::renderMesh(Mesh* mesh, int lod) {
		if (_activeShader == nullptr) return;

		glBindVertexArray(_vertexArrayId);

		if (mesh->getMode() == Mode::Patch) {
			glPatchParameteri(GL_PATCH_VERTICES, mesh->getPatchVertexCount());
		}

		bool needsCompleteUpdate = false;

		if (mesh->_lastShader != _activeShader->getId()) {
			needsCompleteUpdate = true;
			mesh->_enabledVertexAttribArrays = 0;
			mesh->_lastShader = _activeShader->getId();
		}

		mesh->_enabledVertexAttribArrays = 0;
		for (int i = 0; i < 8; i++) {
			VertexBuffer* vb = mesh->getVertexBuffers()[i];
			if (vb == nullptr) continue;
			MeshBuffer::Type type = vb->getType();
			if (type == MeshBuffer::Type::Index || vb->getUsage() == MeshBuffer::Usage::CpuOnly) {
				continue;
			}
			if (!needsCompleteUpdate) {
				//if (!vb->isNeedsUpdate()) continue;
			}

			Attribute* attr = _activeShader->getAttribute(type);
			if (attr->id == -1) {
				attr->id = _activeShader->getAttributeLocation(attr->name);
				if (attr->id == -1) {
					continue;
				}
			}

			GLuint id = vb->getId();

			int slotsRequired = 1;
			if (vb->getNumComponents() > 4) {
				if (vb->getNumComponents() % 4 != 0) {
					std::cerr << "Number of components in multi-slot buffers must be divisible by 4" << std::endl;
				}
				slotsRequired = vb->getNumComponents() / 4;
			}

			if (vb->isNeedsUpdate()) {
				updateBufferData(vb);
			}

			glBindBuffer(GL_ARRAY_BUFFER, vb->getId());

			int slot = attr->id;

			if (slotsRequired == 1) {
				mesh->_vertexAttribArrays[mesh->_enabledVertexAttribArrays++] = slot;
				glVertexAttribPointer(attr->id, vb->getNumComponents(), convertFormat(vb->getFormat()), GL_FALSE, 0, (GLvoid*)0);
				glEnableVertexAttribArray(slot);
				glVertexAttribDivisorARB(attr->id, 0);
			} else {
				for (int i = 0; i < slotsRequired; i++) {
					glVertexAttribPointer(slot + i, 4, convertFormat(vb->getFormat()), GL_FALSE, sizeof(float) * (4 * slotsRequired), (void*)(sizeof(float) * 4 * i));
					mesh->_vertexAttribArrays[mesh->_enabledVertexAttribArrays++] = slot + i;
					glEnableVertexAttribArray(slot + i);
					glVertexAttribDivisorARB(attr->id + i, 1);
				}
			}
		}
		finishRenderMesh(mesh);
	}

	void Renderer::finishRenderMesh(Mesh* mesh) {
		VertexBuffer* ib = mesh->getVertexBuffer(MeshBuffer::Type::Index);
		VertexBuffer* pb = mesh->getVertexBuffer(MeshBuffer::Type::Position);

		for (int i = 0; i < mesh->_enabledVertexAttribArrays; i++) {
			glEnableVertexAttribArray(mesh->_vertexAttribArrays[i]);
		}

		if (ib != nullptr) {
			if (ib->isNeedsUpdate()) {
				updateBufferData(ib);
			}
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->getId());
			if (mesh->getInstanceCount() > 1) {
				glDrawElementsInstanced((GLenum)mesh->getMode(), ib->getBuffer()->size(), convertFormat(ib->getFormat()), 0, mesh->getInstanceCount());
			} else {
				glDrawElements((GLenum)mesh->getMode(), ib->getBuffer()->size(), convertFormat(ib->getFormat()), 0);
			}
		} else if (pb != nullptr) {
			if (mesh->getInstanceCount() > 1) {
				glDrawArraysInstanced((GLenum)mesh->getMode(), 0, pb->getBuffer()->size() / pb->getNumComponents(), mesh->getInstanceCount());
			} else {
				glDrawArrays((GLenum)mesh->getMode(), 0, pb->getBuffer()->size() / pb->getNumComponents());
			}
		}
		for (int i = 0; i < mesh->_enabledVertexAttribArrays; i++) {
			glDisableVertexAttribArray(mesh->_vertexAttribArrays[i]);
		}
	}

	void Renderer::updateBufferData(VertexBuffer* vb) {
		GLuint id = vb->getId();
		if (id == -1) {
			glGenBuffers(1, &id);
			vb->setId(id);
		}

		int target = GL_ARRAY_BUFFER;
		if (vb->getType() == MeshBuffer::Type::Index) {
			target = GL_ELEMENT_ARRAY_BUFFER;
		}
		glBindBuffer(target, id);

		GLenum usage = convertUsage(vb->getUsage());
		Buffer* buffer = vb->getBuffer();
		if (buffer == nullptr) {
			std::cerr << "Buffer cannot be null !" << std::endl;
		}
		switch (buffer->getType()) {
			case BufferType::Float:
			{
				FloatBuffer* fb = ((FloatBuffer*)buffer);
				glBufferData(target, fb->size() * sizeof(float), fb->getData(), usage);
			}
			break;
			case BufferType::Int:
			{
				IntBuffer* ib = ((IntBuffer*)buffer);
				glBufferData(target, ib->size() * sizeof(unsigned int), ib->getData(), usage);
			}
			break;
		}

		vb->markClean();
	}

	GLenum Renderer::convertUsage(MeshBuffer::Usage usage) {
		switch (usage) {
			case MeshBuffer::Usage::Static:
			return GL_STATIC_DRAW;
			case MeshBuffer::Usage::Dynamic:
			return GL_DYNAMIC_DRAW;
			case MeshBuffer::Usage::Stream:
			return GL_STREAM_DRAW;
			default:
			return GL_NONE;
		}
	}

	GLenum Renderer::convertFormat(MeshBuffer::Format format) {
		switch (format) {
			case MeshBuffer::Format::Float:
			return GL_FLOAT;
			case MeshBuffer::Format::Int:
			return GL_UNSIGNED_INT;
			default:
			return GL_NONE;
		}
	}
	#pragma endregion

	TextureCubeMap* Renderer::renderViewPortScene(AssetManager* assetManager, ViewPort* wp, const glm::vec3& pos, RenderManager* rm) {
		int SHADOW_MAP_SIZE = 1024;
		float far_plane = 500.0f;

		Material* depthMat = assetManager->loadMaterial("Assets/Materials/Depth");

		depthMat->setVector3("lightPos", pos);
		depthMat->setFloat("far_plane", far_plane);
		//depthMat->getRenderState().faceCull = FaceCull::Front;

		unsigned int captureFBO;
		glGenFramebuffers(1, &captureFBO);
		//glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

		TextureCubeMap* envMap = new TextureCubeMap(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, ImageFormat::Custom, {});
		envMap->setWrap(WrapMode::ClampToEdge);
		envMap->setMinMagFilter(MinFilter::Nearest, MagFilter::Nearest);

		setTexture(envMap, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envMap->getTextureId());
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, far_plane);
		glm::mat4 captureViews[] = {
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		for (unsigned int i = 0; i < 6; ++i) {
			captureViews[i] = glm::translate(captureViews[i], -pos);
		}

		Camera* cameras[6];
		for (unsigned int i = 0; i < 6; ++i) {
			cameras[i] = new Camera(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
			cameras[i]->setProjectionMatrix(captureProjection);
			cameras[i]->setViewMatrix(captureViews[i]);
		}

		Camera* originalCamera = wp->getCamera();
		rm->setForcedTechniqueDef(depthMat->getTechnique());
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		for (unsigned int i = 0; i < 6; ++i) {
			glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envMap->getTextureId(), 0);
			glClear(GL_DEPTH_BUFFER_BIT);

			wp->setCamera(cameras[i]);
			rm->renderViewPort(wp);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		wp->setCamera(originalCamera);
		rm->setForcedTechniqueDef(nullptr);
		for (unsigned int i = 0; i < 6; ++i) {
			delete cameras[i];
		}

		return envMap;
	}

	TextureCubeMap* Renderer::createPBRMapFromHDR(AssetManager* assetManager, Texture* texture, const PBRMapType& type) {
	#ifdef USE_PBR
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		int camptureSize = 0;

		Material* shader = nullptr;
		if (type == PBRMapType::EnviromentFromHDR) {
			shader = assetManager->loadMaterial("Assets/Materials/PBR/EquirectangularToCubemapShader");
			camptureSize = 512;
		} else if (type == PBRMapType::IrradianceFromEnviroment) {
			shader = assetManager->loadMaterial("Assets/Materials/PBR/IrradianceConvolution");
			camptureSize = 32;
		} else if (type == PBRMapType::PrefilteredMapFromEnviroment) {
			shader = assetManager->loadMaterial("Assets/Materials/PBR/PreFilter");
			camptureSize = 128;
		}

		unsigned int captureFBO;
		unsigned int captureRBO;
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, camptureSize, camptureSize);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		TextureCubeMap* envMap = new TextureCubeMap(camptureSize, camptureSize, ImageFormat::RGB16F, {});
		envMap->setWrap(WrapMode::ClampToEdge);
		if (type == PBRMapType::IrradianceFromEnviroment) {
			envMap->setMinMagFilter(MinFilter::Bilinear, MagFilter::Bilinear);
		} else {
			envMap->setMinMagFilter(MinFilter::Trilinear, MagFilter::Bilinear);
		}
		setTexture(envMap, 0);
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, camptureSize, camptureSize, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		if (type == PBRMapType::PrefilteredMapFromEnviroment) {
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		shader->getRenderState().faceCull = FaceCull::Off;
		if (type == PBRMapType::EnviromentFromHDR) {
			shader->setTexture("EquirectangularMap", texture);
		} else if (type == PBRMapType::IrradianceFromEnviroment || type == PBRMapType::PrefilteredMapFromEnviroment) {
			shader->setTexture("EnvironmentMap", texture);
		}
		shader->setMatrix4("ProjectionMatrix", captureProjection);

		CubeNDC* cube = new CubeNDC();

		glViewport(0, 0, camptureSize, camptureSize); // don't forget to configure the viewport to the capture dimensions.
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

		if (type == PBRMapType::PrefilteredMapFromEnviroment) {
			unsigned int maxMipLevels = 5;
			for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
				unsigned int mipWidth = 128 * std::pow(0.5, mip);
				unsigned int mipHeight = 128 * std::pow(0.5, mip);
				glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
				glViewport(0, 0, mipWidth, mipHeight);

				float roughness = (float)mip / (float)(maxMipLevels - 1);
				shader->setFloat("roughness", roughness);
				for (unsigned int i = 0; i < 6; ++i) {
					shader->setMatrix4("ViewMatrix", captureViews[i]);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envMap->getTextureId(), mip);

					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					applyRenderState(shader->getRenderState());
					shader->render(this, cube);
				}
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		} else {
			if (type == PBRMapType::IrradianceFromEnviroment) {
				glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
			}
			for (unsigned int i = 0; i < 6; ++i) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envMap->getTextureId(), 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				shader->setMatrix4("ViewMatrix", captureViews[i]);
				applyRenderState(shader->getRenderState());
				shader->render(this, cube);
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			setTexture(envMap, 0);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

		delete cube;
		return envMap;
	#endif
		return nullptr;
	}

	Texture* Renderer::createBRDFLUT(AssetManager* assetManager) {
		Material* brdfShader = assetManager->loadMaterial("Assets/Materials/PBR/BrdfLUT");

		unsigned int captureFBO;
		unsigned int captureRBO;
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		Texture2D* texture = new Texture2D(512, 512, ImageFormat::RG16F, nullptr);
		texture->setWrap(WrapMode::ClampToEdge);
		texture->setMinMagFilter(MinFilter::Bilinear, MagFilter::Bilinear);
		setTexture(texture, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);

		brdfShader->getRenderState().faceCull = FaceCull::Off;

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getTextureId(), 0);

		glViewport(0, 0, 512, 512);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		applyRenderState(brdfShader->getRenderState());
		brdfShader->render(this, new QuadBRDF());

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return texture;
	}
}