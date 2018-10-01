#pragma once

#include "glm.h"
#include "vector"
#include "GFXMesh.h"
#include "CLog.h"
#include "StringFormat.h"
#include "FBO.h"
#include "PrimativeGenerator.h"
#include "GLContent.h"
#include "GFXLinker.h"
#include <map>

#define GFX_NULLPTR NULL
#define GFX_NULL_ID -1
#define GFX_RESIZE_NULL 0
#define GFX_RESIZE_LEFT 1
#define GFX_RESIZE_TOP 2
#define GFX_RESIZE_RIGHT 3
#define GFX_RESIZE_BOTTOM 4

namespace gfx
{
	namespace gui
	{
		// TYPEDEFS
		typedef int GFXID;
		typedef glm::vec4 GFXColorStyle[3];
		class GFXManager;

		// MESHES		
		class GFXBoxMesh : public GFXMesh
		{
		public:
			GFXBoxMesh() : GFXMesh() {}
			GFXBoxMesh(glm::vec2 pos, glm::vec2 size) : GFXMesh()
			{
				CINFO("Loading new GFXBox...");
				m_pos = pos;
				m_size = size;
				std::vector<glm::vec3> v = gfx::PrimativeGenerator::generate_square_border(0.02f, size.x / size.y);
				VertexData data = gfx::PrimativeGenerator::pack_object(&v, GEN_COLOR, gfx::BLUE);
				init(data);
			}
		};
		class GFXRectangleMesh : public GFXMesh
		{
		public:
			GFXRectangleMesh() : GFXMesh() {}
			GFXRectangleMesh(glm::vec2 pos, glm::vec2 size) : GFXMesh()
			{
				CINFO("Loading new GFXRectangleMesh...");
				m_pos = pos;
				m_size = size;
				std::vector<glm::vec3> v = gfx::PrimativeGenerator::generate_square_mesh(1, 1);
				VertexData data = gfx::PrimativeGenerator::pack_object(&v, GEN_COLOR, gfx::BLUE);
				init(data);
			}
		};
		class GFXCircleMesh : public GFXMesh
		{
		public:
			GFXCircleMesh() : GFXMesh() {}
			GFXCircleMesh(glm::vec2 pos, glm::vec2 size) : GFXMesh()
			{
				CINFO("Loading new GFXCircleMesh...");
				m_pos = pos;
				m_size = size;
				std::vector<glm::vec3> v = gfx::PrimativeGenerator::generate_circle(20);
				VertexData data = gfx::PrimativeGenerator::pack_object(&v, GEN_COLOR, gfx::BLUE);
				init(data);
			}
		};

		// BASE CLASSES //
		class GFXUnit
		{
		public:
			GFXManager * getManager()
			{
				return m_manager;
			}
			void setManager(GFXManager * manager)
			{
				m_manager = manager;
			}

			GFXID getId()
			{
				return m_id;
			}
			void setId(int id)
			{
				m_id = id;
			}

			GFXUnit()
			{
				m_id = GFX_NULL_ID;
				m_manager = GFX_NULLPTR;
			}
			GFXUnit(GFXID id, GFXManager * manager)
			{
				m_id = id;
				m_manager = manager;
			}
			GFXUnit(GFXUnit * component)
			{
				m_id = component->m_id;
				m_manager = component->m_manager;
			}

		protected:
			GFXManager * m_manager;
			GFXID m_id;
		};
		
		class GFXComponent : public GFXUnit, public GFXMesh
		{
		public:
			bool isInside(gfx::engine::GLContent * content)
			{
				if (content != nullptr)
				{
					glm::vec2 mousePos = getRelativeMousePos(content);
					return mousePos.x >= 0 && mousePos.y >= 0 && mousePos.x <= m_size.x && mousePos.y <= m_size.y;
				}
				return false;
			}

			glm::vec2 getRelativeMousePos(gfx::engine::GLContent * content)
			{
				glm::vec2 mousePos = content->getMousePos();
				GFXComponent * parent = m_parent;
				while (parent != GFX_NULLPTR)
				{
					mousePos -= parent->m_pos;
					parent = m_parent->getParent();
				}
				mousePos -= m_pos;
				return mousePos;
			}

			GFXComponent * getParent()
			{
				return m_parent;
			}
			void setParent(GFXComponent * parent)
			{
				m_parent = parent;
			}

			virtual GFXComponent * init() = 0;
			virtual void validate() = 0;
			virtual void update(gfx::engine::GLContent * content) = 0;
			virtual bool checkEvents(gfx::engine::GLContent * content) = 0;
			virtual void draw(gfx::engine::VarHandle *model, gfx::engine::VarHandle *color) = 0;
			virtual void draw(glm::mat4 modelMat, gfx::engine::VarHandle *model, gfx::engine::VarHandle *color) = 0;

			GFXComponent * setColorStyle(GFXColorStyle colorStyle)
			{
				m_colorStyle[0] = colorStyle[0];
				m_colorStyle[1] = colorStyle[1];
				m_colorStyle[2] = colorStyle[2];
				return this;
			}

			void inheritColorStyle(GFXComponent * parent)
			{
				if (parent != GFX_NULLPTR)
				{
					m_colorStyle[0] = parent->m_colorStyle[0];
					m_colorStyle[1] = parent->m_colorStyle[1];
					m_colorStyle[2] = parent->m_colorStyle[2];
				}
			}

			GFXComponent()
			{
				inheritColorStyle(m_parent);
			}
			GFXComponent(GFXMesh mesh) : GFXUnit(), GFXMesh(mesh)
			{
				inheritColorStyle(m_parent);
			}
		protected:
			GFXComponent * m_parent = GFX_NULLPTR;
			GFXColorStyle m_colorStyle = { gfx::CYAN_A, gfx::WHITE_A, gfx::GREY_A };
		};

		class GFXGroup
		{
		public:
			GFXGroup * add(GFXComponent * component, int id, GFXComponent * parent, GFXManager * manager)
			{
				component->setId(id);
				component->setParent(parent);
				component->setManager(manager);
				component->inheritColorStyle(parent);
				component->init();
				m_group.push_back(component);
				return this;
			}

			int getComponent(GFXComponent * component)
			{
				for (int ix = 0; ix < m_group.size(); ++ix)
					if (m_group[ix] == component)
						return ix;
				return GFX_NULL_ID;
			}

			bool removeComponent(GFXComponent * component)
			{
				int id = getComponent(component);
				if (id != GFX_NULL_ID)
				{
					m_group.erase(m_group.begin() + id);
					return true;
				}

				bool success = false;
				for (GFXComponent * c : m_group)
					if (typeid(GFXGroup) == typeid(c))
						success |= dynamic_cast<GFXGroup*>(c)->removeComponent(component);
				return success;
			}

			void drawGroup(gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				for (GFXComponent * component : m_group)
					component->draw(model, color);
			}
			void drawGroup(glm::mat4 modelMat, gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				for (GFXComponent * component : m_group)
					component->draw(modelMat, model, color);
			}

			bool checkGroupEvents(gfx::engine::GLContent * content)
			{
				for (int ix = m_group.size() - 1; ix >= 0; --ix)
					if (m_group[ix]->checkEvents(content))
						return true;
				return false;
			}

			void updateGroup(gfx::engine::GLContent * content)
			{
				for (GFXComponent * component : m_group)
					component->update(content);
			}

			void initGroup()
			{
				for (GFXComponent * component : m_group)
					component->init();
			}

			void validateGroup()
			{
				for (GFXComponent * component : m_group)
					component->validate();
			}

			void bringForward(GFXComponent * component)
			{
				for (int i = 0; i < m_group.size(); ++i)
					if (m_group[i] == component)
					{
						if (i != m_group.size() - 1)
						{
							m_group.erase(m_group.begin() + i);
							m_group.insert(m_group.begin() + i + 1, component);
						}
						break;
					}
			}
			void bringBackward(GFXComponent * component)
			{
				for (int i = 0; i < m_group.size(); ++i)
					if (m_group[i] == component)
					{
						if (i != 0)
						{
							m_group.erase(m_group.begin() + i);
							m_group.insert(m_group.begin() + i - 1, component);
						}
						break;
					}
			}
		private:
			std::vector<GFXComponent*> m_group;
		};


		// MANAGERS
		class GFXManager
		{
		public:
			gfx::engine::FBOID getFBOId()
			{
				return m_fboId;
			}
			void setFBOId(gfx::engine::FBOID id)
			{
				m_fboId = id;
			}

			gfx::engine::GLSLProgramID getProgramId()
			{
				return m_programId;
			}
			void setProgramId(gfx::engine::GLSLProgramID id)
			{
				m_programId = id;
			}

			gfx::engine::VarHandle getColorHandle()
			{
				return m_colorHandle;
			}
			void setColorHandle(gfx::engine::VarHandle colorHandle)
			{
				m_colorHandle = colorHandle;
			}

			int getNextId()
			{
				return currentId++;
			}

			GFXManager * addComponent(GFXComponent * component)
			{
				m_group.add(component, getNextId(), GFX_NULLPTR, this);
				component->init();
				return this;
			}

			bool removeComponent(GFXComponent * component)
			{
				return m_group.removeComponent(component);
			}

			void checkEvents(gfx::engine::GLContent * content)
			{
				m_group.checkGroupEvents(content);
			}
			void update(gfx::engine::GLContent * content)
			{
				m_group.updateGroup(content);
			}

			void draw(gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				m_group.drawGroup(model, color);
			}

			void init()
			{
				m_group.initGroup();
			}

			GFXManager() {};
			GFXManager(gfx::engine::GLSLProgramID programId, gfx::engine::FBOID fboId)
			{
				m_fboId = fboId;
				m_programId = programId;
			}
		protected:
			gfx::engine::FBOID m_fboId;
			gfx::engine::GLSLProgramID m_programId;
			gfx::engine::VarHandle m_colorHandle;
			gfx::gui::GFXGroup m_group;
			int currentId = -1;
		};

		class GFXContainer : public GFXComponent, public GFXGroup
		{
		public:
			GFXContainer * addComponent(GFXComponent * component)
			{
				add(component, m_manager->getNextId(), this, m_manager);
				return this;
			}

			GFXComponent * init()
			{
				initGroup();
				return this;
			}

			void validate()
			{
				validateGroup();
			}

			void update(gfx::engine::GLContent * content)
			{
				updateGroup(content);
			}

			bool checkEvents(gfx::engine::GLContent * content)
			{
				return checkGroupEvents(content);
			}

			void draw(gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				drawMesh(model, color);
				drawGroup(getRelativeModelMat(), model, color);
			}
			void draw(glm::mat4 modelMat, gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				drawMesh(modelMat, model, color);
				drawGroup(modelMat * getRelativeModelMat(), model, color);
			}

			GFXContainer() : GFXComponent() {}
			GFXContainer(GFXMesh mesh) : GFXComponent(mesh) {}
		private:
		};


		// CLICKABLES
		class GFXClickable : public GFXComponent
		{
		public:
			bool onPressed(bool isTrue)
			{
				if (isTrue)
				{
					m_onDown = true;
				}
				return isTrue;
			}
			bool onDown(bool isTrue)
			{
				if (isTrue)
				{
					m_onDown = true;
				}
				return isTrue;
			}
			bool onReleased(bool isTrue)
			{
				if (isTrue)
				{
					m_onDown = false;
					m_onReleased = true;
					m_onDragging = false;
				}
				else
					m_onReleased = false;
				return isTrue;
			}
			bool onDragging(bool isTrue)
			{
				return m_onDragging = isTrue;
			}

			bool isPressed(gfx::engine::GLContent * content)
			{
				return content->getKeyboardEvents()->isKeyPressed(VK_LBUTTON) &&
					isInside(content);
			}
			bool isDown(gfx::engine::GLContent * content)
			{
				return content->getKeyboardEvents()->isKeyDown(VK_LBUTTON) &&
					content->getKeyboardEvents()->isKeyPressed(VK_LBUTTON) &&
					isInside(content);
			}
			bool isReleased(gfx::engine::GLContent * content)
			{
				return content->getKeyboardEvents()->isKeyReleased(VK_LBUTTON) &&
					m_onDown;
			}
			bool isDragging(gfx::engine::GLContent * content)
			{
				return m_onDown &&
					glm::length(content->getMouseDelta()) > 0;
			}

			GFXClickable() {}
			GFXClickable(GFXMesh mesh) : GFXComponent(mesh) {}

			bool m_onDragging = false;
			bool m_onDown = false;
			bool m_onReleased = false;
		};

		class GFXClickableContainer : public GFXClickable, public GFXGroup
		{
		public:
			GFXClickableContainer * addComponent(GFXComponent * component)
			{
				add(component, m_manager->getNextId(), this, m_manager);
				return this;
			}

			GFXComponent * init()
			{
				initGroup();
				return this;
			}

			void validate()
			{
				validateGroup();
			}

			void update(gfx::engine::GLContent * content)
			{
				updateGroup(content);
			}

			bool checkEvents(gfx::engine::GLContent * content)
			{
				onPressed(isPressed(content));
				onDown(isDown(content));
				onReleased(isReleased(content));
				onDragging(isDragging(content));
				return checkGroupEvents(content) || m_onDown;
			}

			void draw(gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				drawMesh(model, color);
				drawGroup(getRelativeModelMat(), model, color);
			}
			void draw(glm::mat4 modelMat, gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				drawMesh(modelMat, model, color);
				drawGroup(modelMat * getRelativeModelMat(), model, color);
			}

			GFXClickableContainer(GFXMesh mesh) : GFXClickable(mesh) {}
		private:
		};

		class GFXButtonRect : public GFXClickable, public GFXLinker<GFXButtonRect>
		{
		public:

			void update(gfx::engine::GLContent * content)
			{
				m_back->setPos(m_pos);
			}
			bool checkEvents(gfx::engine::GLContent * content)
			{
				onButtonPressed(content);
				onButtonDown(content);
				onButtonReleased(content);
				onButtonDragging(content);
				return m_onDown;
			}
			void draw(gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				m_back->setPos(m_pos);
				m_back->drawMesh(model, color);
			}
			void draw(glm::mat4 modelMat, gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				m_back->setPos(m_pos);
				m_back->drawMesh(modelMat, model, color);
			}
		
			GFXComponent * init()
			{
				m_back = new GFXRectangleMesh(m_pos, m_size);
				m_back->setColor(m_colorStyle[0]);
				return this;
			}

			void validate()
			{
				m_back->setPos(m_pos);
				m_back->setSize(m_size);
			}

			GFXButtonRect(glm::vec2 pos, glm::vec2 size)
			{
				m_pos = pos;
				m_size = size;
			}				

			void onButtonPressed(gfx::engine::GLContent * content)
			{
				if (onPressed(isPressed(content)))
				{
					m_back->setColor(m_colorStyle[0]);
					callTrigger(&GFXButtonRect::onButtonPressed);
				}
			}
			void onButtonDown(gfx::engine::GLContent * content)
			{
				if (onDown(isDown(content)))
				{
					m_back->setColor(glm::vec3(m_colorStyle[0]) / 2.0f);
					callTrigger(&GFXButtonRect::onButtonDown);
				}
			}
			void onButtonReleased(gfx::engine::GLContent * content)
			{
				if (onReleased(isReleased(content)))
				{
					m_back->setColor(m_colorStyle[0]);
					callTrigger(&GFXButtonRect::onButtonReleased);
				}
			}
			void onButtonDragging(gfx::engine::GLContent * content)
			{
				if (onDragging(isDragging(content)))
				{
					m_back->setColor(glm::vec3(m_colorStyle[0]) / 1.5f);
					callTrigger(&GFXButtonRect::onButtonDragging);
				}
			}
		protected:
			GFXRectangleMesh * m_back;
		};

		class GFXButtonCircle : public GFXClickable, public GFXLinker<GFXButtonCircle>
		{
		public:

			void update(gfx::engine::GLContent * content)
			{
				m_back->setPos(m_pos);
			}
			bool checkEvents(gfx::engine::GLContent * content)
			{
				onButtonPressed(content);
				onButtonDown(content);
				onButtonReleased(content);
				onButtonDragging(content);
				return m_onDown;
			}
			void draw(gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				m_back->setPos(m_pos);
				m_back->drawMesh(model, color);
			}
			void draw(glm::mat4 modelMat, gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				m_back->setPos(m_pos);
				m_back->drawMesh(modelMat, model, color);
			}

			GFXComponent * init()
			{
				m_back = new GFXCircleMesh(m_pos, m_size);
				m_back->setColor(m_colorStyle[0]);
				return this;
			}

			void validate()
			{
				m_back->setPos(m_pos);
				m_back->setSize(m_size);
			}

			GFXButtonCircle(glm::vec2 pos, glm::vec2 size)
			{
				m_pos = pos;
				m_size = size;		
			}

			void onButtonPressed(gfx::engine::GLContent * content)
			{
				if (onPressed(isPressed(content)))
				{
					m_back->setColor(m_colorStyle[0]);
					callTrigger(&GFXButtonCircle::onButtonPressed);
				}
			}
			void onButtonDown(gfx::engine::GLContent * content)
			{
				if (onDown(isDown(content)))
				{
					m_back->setColor(glm::vec3(m_colorStyle[0]) / 2.0f);
					callTrigger(&GFXButtonCircle::onButtonDown);
				}
			}
			void onButtonReleased(gfx::engine::GLContent * content)
			{
				if (onReleased(isReleased(content)))
				{
					m_back->setColor(m_colorStyle[0]);
					callTrigger(&GFXButtonCircle::onButtonReleased);
				}
			}
			void onButtonDragging(gfx::engine::GLContent * content)
			{
				if (onDragging(isDragging(content)))
				{
					m_back->setColor(glm::vec3(m_colorStyle[0]) / 1.5f);
					callTrigger(&GFXButtonCircle::onButtonDragging);
				}
			}
		protected:
			GFXCircleMesh * m_back;
		};

		class GFXWindow : public GFXClickable, public GFXLinker<GFXWindow>
		{
		public:

			void update(gfx::engine::GLContent * content)
			{
				if (m_bar->m_onDragging)
				{
					m_pos += content->getMouseDelta();
					m_pos.x = min(content->getWindowSize().x - m_size.x, m_pos.x);
					m_pos.x = max(0, m_pos.x);
					m_pos.y = min(content->getWindowSize().y - m_size.y, m_pos.y);
					m_pos.y = max(0, m_pos.y);
					m_container->setPos(m_pos);
				}

				if (m_resizingDir != GFX_RESIZE_NULL)
				{
					glm::vec2 delta = content->getMouseDelta();
					switch (m_resizingDir)
					{
					case GFX_RESIZE_LEFT:						
						m_pos.x += delta.x;
						m_size.x -= delta.x;
						break;
					case GFX_RESIZE_RIGHT:
						m_size.x += delta.x;
						break;
					case GFX_RESIZE_BOTTOM:
						m_pos.y += delta.y;
						m_size.y -= delta.y;
						break;
					}

					validate();
				}

				m_container->update(content);
			}
			bool checkEvents(gfx::engine::GLContent * content)
			{
				onWindowPressed(content);
				onWindowDown(content);
				onWindowReleased(content);
				onWindowDragging(content);
				onWindowMove(content);

				onClose(content);
				onToggleMaximise(content);
				onResize(content);
				return m_container->checkEvents(content) || m_onDown;
			}
			void draw(gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				m_container->draw(model, color);
			}
			void draw(glm::mat4 modelMat, gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				m_container->draw(modelMat, model, color);
			}

			GFXWindow * addComponent(GFXComponent * component)
			{
				m_container->addComponent(component);
				return this;
			}

			void validate()
			{
				glm::vec2 pos = m_pos, size = m_size;

				m_container->setPos(pos);
				m_container->setSize(size);

				pos.x = 0;
				pos.y = size.y - 20;
				size.y = 20;
				m_bar->setPos(pos);
				m_bar->setSize(size);

				pos.x = size.x - 20;
				size.x = 20;
				size.y = 20;
				m_close->setPos(pos);
				m_close->setSize(size);


				pos.x -= 20;
				m_maxmin->setPos(pos);
				m_maxmin->setSize(size);

				m_container->validate();
			}

			GFXComponent * init()
			{
				glm::vec2 pos = m_pos, size = m_size;

				std::vector<glm::vec3> v = gfx::PrimativeGenerator::generate_square_mesh(1, 1);
				m_container = new gfx::gui::GFXClickableContainer(gfx::gui::GFXMesh(
					pos,
					size,
					gfx::PrimativeGenerator::pack_object(&v, GEN_COLOR, gfx::WHITE),
					m_colorStyle[2]));
				m_container->setAlpha(0.8f);
				m_container->setManager(m_manager);
				m_container->setColorStyle(m_colorStyle);

				pos.x = 0;
				pos.y = size.y - 20;
				size.y = 20;
				m_bar = new gfx::gui::GFXButtonRect(pos, size);
				addComponent(m_bar);

				pos.x = size.x - 20;
				size.x = 20;
				size.y = 20;
				m_close = new gfx::gui::GFXButtonRect(pos, size);
				addComponent(m_close);
				GFXColorStyle colorStyle = { gfx::RED_A, m_colorStyle[1], m_colorStyle[2] };
				m_close->setColorStyle(colorStyle);

				pos.x -= 20;
				m_maxmin = new gfx::gui::GFXButtonRect(pos, size);
				addComponent(m_maxmin);
				colorStyle[0] = gfx::BLUE_A;
				m_maxmin->setColorStyle(colorStyle);

				m_container->init();

				return this;
			}

			GFXWindow(glm::vec2 pos, glm::vec2 size)
			{
				m_size = size;
				m_pos = pos;
			}

			void onWindowPressed(gfx::engine::GLContent * content)
			{
				if (onPressed(isPressed(content)))
				{
					callTrigger(&GFXWindow::onWindowPressed);
				}
			}
			void onWindowDown(gfx::engine::GLContent * content)
			{
				if (onDown(isDown(content)))
				{
					callTrigger(&GFXWindow::onWindowDown);
				}
			}
			void onWindowReleased(gfx::engine::GLContent * content)
			{
				if (onReleased(isReleased(content)))
				{
					callTrigger(&GFXWindow::onWindowReleased);
				}
			}
			void onWindowDragging(gfx::engine::GLContent * content)
			{
				if (onDragging(isDragging(content)))
				{
					callTrigger(&GFXWindow::onWindowDragging);
				}
			}
			void onWindowMove(gfx::engine::GLContent * content)
			{
				if (m_onDragging)
				{
					callTrigger(&GFXWindow::onWindowMove);
				}
			}

			void closeWindow()
			{
				if (!m_manager->removeComponent(this))
				{
					CERROR("failed to close window.", __FILE__, __LINE__, "GFXWindow", __func__);
				}
			}
			void onClose(gfx::engine::GLContent * content)
			{
				if (m_close->isPressed(content))
				{
					callTrigger(&GFXWindow::onClose);
					closeWindow();
				}
			}

			void maximiseWindow(gfx::engine::GLContent * content)
			{
				m_oldPos = m_pos;
				m_oldSize = m_size;

				m_pos = glm::vec2();
				m_size = content->getWindowSize();

				validate();
			}
			void minimiseWindow()
			{
				m_pos = m_oldPos;
				m_size = m_oldSize;

				validate();
			}
			void toggleMaximise(gfx::engine::GLContent * content)
			{
				m_maximised = !m_maximised;
				if (m_maximised)
					maximiseWindow(content);
				else
					minimiseWindow();
			}
			void onToggleMaximise(gfx::engine::GLContent * content)
			{
				if (m_maxmin->isPressed(content))
				{
					toggleMaximise(content);
					callTrigger(&GFXWindow::onToggleMaximise);
				}
			}

			void onResize(gfx::engine::GLContent * content)
			{
				if (m_onDragging)
				{
					glm::vec2 mousePos = getRelativeMousePos(content);
					float deadzone = 10;
					if (mousePos.x >= 0 && mousePos.x <= deadzone && mousePos.y >= 0 && mousePos.y <= m_size.y)
						m_resizingDir = GFX_RESIZE_LEFT;
					else if (mousePos.x >= -deadzone + m_size.x && mousePos.x <= m_size.x && mousePos.y >= 0 && mousePos.y <= m_size.y)
						m_resizingDir = GFX_RESIZE_RIGHT;
					else if (mousePos.x >= 0 && mousePos.x <= m_size.x && mousePos.y >= 0 && mousePos.y <= deadzone)
						m_resizingDir = GFX_RESIZE_BOTTOM;
					else
						m_resizingDir = GFX_RESIZE_NULL;

					callTrigger(&GFXWindow::onResize);
				}
				else
					m_resizingDir = GFX_RESIZE_NULL;
			}
		protected:
			bool m_windowMoving = false;
			int m_resizingDir = GFX_RESIZE_NULL;
			bool m_maximised = false;
			glm::vec2 m_oldPos;
			glm::vec2 m_oldSize;
			GFXButtonRect * m_bar;
			GFXButtonRect * m_close;
			GFXButtonRect * m_maxmin;
			GFXClickableContainer * m_container;
		};

		class GFXSlider : public GFXClickable, public GFXLinker<GFXSlider>
		{
		public:
			void update(gfx::engine::GLContent * content)
			{
				if (m_pin->m_onDragging)
				{
					m_pin->m_pos.x += content->getMouseDelta().x;
					m_pin->m_pos.x = max(m_pin->m_pos.x, m_size.y / 2);
					m_pin->m_pos.x = min(m_pin->m_pos.x, m_size.x - m_size.y / 2);
				}
				if(m_pin->m_onReleased)
					if (m_snapToValues)
					{
						m_pin->m_pos.x = round(getValue() / m_snapInterval) / (1/m_snapInterval) * (m_size.x - m_size.y) + m_size.y / 2;
					}
			}
			bool checkEvents(gfx::engine::GLContent * content)
			{
				onSliderPressed(content);
				onSliderDown(content);
				onSliderReleased(content);
				onSliderDragging(content);
				m_pin->checkEvents(content);
				onSlide(content);
				return m_onDown;
			}
			void draw(gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				m_back->drawMesh(getRelativeModelMat(), model, color);
				m_rail->drawMesh(getRelativeModelMat(), model, color);
				m_pin->draw(getRelativeModelMat(), model, color);
			}
			void draw(glm::mat4 modelMat, gfx::engine::VarHandle *model, gfx::engine::VarHandle *color)
			{
				m_back->drawMesh(modelMat* getRelativeModelMat(), model, color);
				m_rail->drawMesh(modelMat* getRelativeModelMat(), model, color);
				m_pin->draw(modelMat* getRelativeModelMat(), model, color);
			}

			float getValue()
			{
				return (m_pin->m_pos.x - m_size.y / 2) / (m_size.x - m_size.y);
			}

			GFXComponent * init()
			{
				float radius = m_size.y / 2;
				m_back = new GFXRectangleMesh(glm::vec2(), m_size);
				m_back->setColor(m_colorStyle[2]);
				m_rail = new GFXRectangleMesh(glm::vec2(radius, radius), glm::vec2(m_size.x - radius * 2, 2));
				m_rail->setColor(m_colorStyle[1]);
				m_pin = new GFXButtonCircle(glm::vec2(radius, radius), glm::vec2(radius, radius));
				m_pin->setId(m_manager->getNextId());
				m_pin->setParent(this);
				m_pin->setManager(m_manager);
				m_pin->inheritColorStyle(this);
				m_pin->init();
				return this;
			}

			void validate()
			{
				float radius = m_size.y / 2;
				m_back->setPos(glm::vec2());
				m_back->setSize(m_size);

				m_rail->setPos(glm::vec2(radius, radius));
				m_rail->setSize(glm::vec2(m_size.x - radius * 2, 2));

				m_pin->setPos(glm::vec2(round(getValue() / m_snapInterval) / (1 / m_snapInterval) * (m_size.x - m_size.y) + m_size.y / 2, radius));
				m_pin->setSize(glm::vec2(radius, radius));
				m_pin->validate();
			}

			GFXSlider(glm::vec2 pos, glm::vec2 size, bool snapToValues, float interval)
			{
				m_snapToValues = snapToValues;
				m_snapInterval = interval;
				m_pos = pos;
				m_size = size;
			}

			void onSliderPressed(gfx::engine::GLContent * content)
			{
				if(onPressed(isPressed(content)))
				{
					callTrigger(&GFXSlider::onSliderPressed);
				}
			}
			void onSliderDown(gfx::engine::GLContent * content)
			{
				if(onDown(isDown(content)))
				{
					callTrigger(&GFXSlider::onSliderDown);
				}
			}
			void onSliderReleased(gfx::engine::GLContent * content)
			{
				if(onReleased(isReleased(content)))
				{
					callTrigger(&GFXSlider::onSliderReleased);
				}
			}
			void onSliderDragging(gfx::engine::GLContent * content)
			{
				if (onDragging(isReleased(content)))
				{
					callTrigger(&GFXSlider::onSliderDragging);
				}
			}

			void onSlide(gfx::engine::GLContent * content)
			{
				if (m_pin->m_onDragging)
				{
					callTrigger(&GFXSlider::onSlide);
				}
			}
		protected:
			bool m_snapToValues;
			float m_snapInterval;

			GFXButtonCircle * m_pin;
			GFXRectangleMesh * m_rail;
			GFXRectangleMesh * m_back;
		};

	}
}