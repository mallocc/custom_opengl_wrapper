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
#include "ImageLoader.h"
#include <map>

#define GFX_NULLPTR NULL
#define GFX_NULL_ID -1
#define GFX_RESIZE_NULL 0
#define GFX_RESIZE_LEFT 1
#define GFX_RESIZE_TOP 2
#define GFX_RESIZE_RIGHT 3
#define GFX_RESIZE_BOTTOM 4

#define GFX_GUI_SHADER_BLOCK 0
#define GFX_GUI_SHADER_FONT 1
#define GFX_GUI_SHADER_TEXTURE 2
#define GFX_GUI_DEFAULT_FONT_SIZE 20
#define GFX_GUI_DEFAULT_PADDING 25

namespace gfx
{
    namespace gui
	{

		const char * GFX_Courier_FONT = "textures/gfx_default.png";
		const char * GFX_Malgun_Gothic_FONT = "textures/Malgun_Gothic.png";

		// TYPEDEFS
		typedef int GFXID;
		typedef glm::vec4 GFXColorStyle[3];
		class GFXManager;

		class GFXFont
		{
		public:
			// Draws the mesh including linking the model matrix
			void draw(unsigned char c, glm::vec2 pos, glm::vec2 scale, gfx::engine::MeshHandle_T handles)
			{
				handles.modelMatHandle->load(glm::translate(glm::mat4(1.), glm::vec3(pos,0)) *
					glm::scale(glm::mat4(1.), glm::vec3(scale,0)));
				handles.colorHandle->load(m_color);
				handles.flagHandle->load(GFX_GUI_SHADER_FONT);
				drawArray(c, handles.textureHandle);
			}
			// Draws the mesh including linking the model matrix
			void draw(unsigned char c, glm::mat4 modelMat, glm::vec2 pos, glm::vec2 scale, gfx::engine::MeshHandle_T handles)
			{
				handles.modelMatHandle->load(modelMat * glm::translate(glm::mat4(1.), glm::vec3(pos, 0)) *
					glm::scale(glm::mat4(1.), glm::vec3(scale, 0)));
				handles.colorHandle->load(m_color);
				handles.flagHandle->load(GFX_GUI_SHADER_FONT);
				drawArray(c, handles.textureHandle);
			}

			// Draws just the VBO and activating the texture
			void drawArray(unsigned char c, gfx::engine::VarHandle *textureHandle)
			{
				// load the textures
				if (m_tex != GL_TEXTURE0)
				{
					loadTextureHandle(textureHandle);
					glActiveTexture(GL_TEXTURE0 + m_tex);
					glBindTexture(GL_TEXTURE_2D, m_tex);
				}

				// draw the data
				glBindVertexArray(m_vao);
				glDrawArrays(GL_TRIANGLES, c * 6, 6);
				glBindVertexArray(0);

				// unload the texture
				if (m_tex != GL_TEXTURE0)
				{
					glActiveTexture(GL_TEXTURE0 + m_tex);
					glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);
				}
				glActiveTexture(GL_TEXTURE0);
				glFinish();
			}

			// Override the texture handle seperately
			void loadTextureHandle(gfx::engine::VarHandle * handle)
			{
				handle->load(m_tex);
			}

			// Loads image file into a texture
			void loadTextures(const char *texfilename)
			{
				if (texfilename != "")
				{
					m_tex = alib::ImageLoader::loadTextureFromImage(texfilename);
					CINFO(alib::StringFormat("    %0 -> Texture ID %1").arg(texfilename).arg(m_tex).str());
				}
				else
				{
					CINFO("    no texture file loaded");
				}
			}

			// Buffers Vertex data into the VBO
			void init(gfx::VertexData d)
			{
				m_dataSize = d.size();
				glGenVertexArrays(1, &m_vao);
				glBindVertexArray(m_vao);
				glGenBuffers(1, &m_buffer);
				glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
				glBufferData(GL_ARRAY_BUFFER, m_dataSize * sizeof(struct gfx::Vertex_T), d.data(), GL_STATIC_DRAW);
				glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(struct gfx::Vertex_T),
					(const GLvoid*)offsetof(struct gfx::Vertex_T, position));
				glEnableVertexAttribArray(0);
				glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(struct gfx::Vertex_T),
					(const GLvoid*)offsetof(struct gfx::Vertex_T, color));
				glEnableVertexAttribArray(1);
				glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, sizeof(struct gfx::Vertex_T),
					(const GLvoid*)offsetof(struct gfx::Vertex_T, normal));
				glEnableVertexAttribArray(2);
				glVertexAttribPointer((GLuint)3, 2, GL_FLOAT, GL_FALSE, sizeof(struct gfx::Vertex_T),
					(const GLvoid*)offsetof(struct gfx::Vertex_T, uv));
				glEnableVertexAttribArray(3);
				glVertexAttribPointer((GLuint)4, 3, GL_FLOAT, GL_FALSE, sizeof(struct gfx::Vertex_T),
					(const GLvoid*)offsetof(struct gfx::Vertex_T, tangent));
				glEnableVertexAttribArray(4);
				glBindVertexArray(0);
				glFlush();

				CINFO(alib::StringFormat("    buffered into VAO %0").arg(m_vao).str());
			}

			void setColor(glm::vec4 color)
			{
				m_color = color;
			}
			void setColor(glm::vec3 color)
			{
				m_color = glm::vec4(color, 1);
			}
			
			GFXFont()
			{
			}
			GFXFont(const char *fontfile)
			{
				gfx::VertexData o = 
					gfx::PrimativeGenerator::pack_object(
						&gfx::PrimativeGenerator::generate_square_meshes(256),
						GEN_SQUAREUVS, gfx::WHITE);
				loadTextures(fontfile);
				init(o);
			}
		private:
			GLuint
				m_vao,
				m_buffer,
				m_tex = GL_TEXTURE0;
			int
				m_dataSize = 0;
			glm::vec4 m_color = gfx::WHITE_A;
		};

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
				std::vector<glm::vec3> v = gfx::PrimativeGenerator::generate_centered_circle(20);
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
					parent = parent->getParent();
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
			virtual void draw(gfx::engine::MeshHandle_T handles) = 0;
			virtual void draw(glm::mat4 modelMat, gfx::engine::MeshHandle_T handles) = 0;

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

			void drawGroup(gfx::engine::MeshHandle_T handles)
			{
				for (GFXComponent * component : m_group)
					component->draw(handles);
			}
			void drawGroup(glm::mat4 modelMat, gfx::engine::MeshHandle_T handles)
			{
				for (GFXComponent * component : m_group)
					component->draw(modelMat, handles);
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

			glm::vec2 getMinimumBounds(float padding)
			{
				float left = 1000000;
				float right = 0.0f;
				float top = 0.0f;
				float bottom = 1000000;
				for (GFXComponent * component : m_group)
				{
					glm::vec2 size = component->getSize();
					glm::vec2 pos = component->getPos();
					left = min(pos.x, left);
					right = max(pos.x + size.x, right);
					bottom = min(pos.y, bottom);
					top = max(pos.y + size.y, top);
				}
				return glm::vec2(right + padding, top + padding);
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


		class GFXLabel : public GFXComponent
		{
		public:
			
			GFXComponent * init()
			{
				m_font = GFXFont(GFX_Courier_FONT);
				return this;
			}
			void validate()
			{

			}
			void update(gfx::engine::GLContent * content)
			{

			}
			bool checkEvents(gfx::engine::GLContent * content)
			{
				return false;
			}
			void draw(gfx::engine::MeshHandle_T handles)
			{
				for (int ix = 0; ix < m_text.length(); ix++)
					m_font.draw(m_text[ix], glm::vec2(ix * m_size.x/2, m_size.y/2) + m_pos, m_size, handles);
			}
			void draw(glm::mat4 modelMat, gfx::engine::MeshHandle_T handles)
			{
				for (int ix = 0; ix < m_text.length(); ix++)
					m_font.draw(m_text[ix], modelMat, glm::vec2(ix * m_size.x/2, m_size.y/2) + m_pos, m_size, handles);
			}

			float getLength()
			{
				return m_size.x / 2 * m_text.size();
			}

			void center()
			{
				m_pos -= glm::vec2(getLength() / 2, m_size.y);
			}

			void setText(std::string text)
			{
				m_text = text;
			}

			void setColor(glm::vec4 color)
			{
				m_font.setColor(color);
			}

			GFXLabel(std::string text, int fontSize)
			{
				m_text = text;
				m_size = glm::vec2(fontSize, fontSize);
			}

		protected:
			GFXFont m_font;
			std::string m_text;
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

			void draw(gfx::engine::MeshHandle_T handles)
			{
				m_group.drawGroup(handles);
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

			void draw(gfx::engine::MeshHandle_T handles)
			{
				drawMesh(handles);
				drawGroup(getRelativeModelMat(), handles);
			}
			void draw(glm::mat4 modelMat, gfx::engine::MeshHandle_T handles)
			{
				drawMesh(modelMat, handles);
				drawGroup(modelMat * getRelativeModelMat(), handles);
			}

			GFXContainer() : GFXComponent() {}
			GFXContainer(glm::vec2 pos, glm::vec2 size, glm::vec4 color) : GFXComponent() 
			{
				inheritColorStyle(m_parent);
				std::vector<glm::vec3> v = gfx::PrimativeGenerator::generate_square_mesh(1, 1);
				GFXMesh miniMesh = gfx::gui::GFXMesh(
					pos,
					size,
					gfx::PrimativeGenerator::pack_object(&v, GEN_COLOR, gfx::WHITE),
					color);
				m_pos = miniMesh.m_pos;
				m_size = miniMesh.m_size;
				m_vao = miniMesh.m_vao;
				m_buffer = miniMesh.m_buffer;
				m_dataSize = miniMesh.m_dataSize;
				m_rotation = miniMesh.m_rotation;
				m_theta = miniMesh.m_theta;
				m_color = miniMesh.m_color;
			}
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
					m_heldCounter++;
					m_onDown = true;
				}
				return isTrue;
			}
			bool onReleased(bool isTrue)
			{
				if (isTrue)
				{
					m_heldCounter = 0;
					m_onDown = false;
					m_onReleased = true;
				}
				else
					m_onReleased = false;
				return isTrue;
			}

			bool isPressed(gfx::engine::GLContent * content)
			{
				return content->getKeyboardEvents()->isKeyPressed(VK_LBUTTON) &&
					isInside(content);
			}
			bool isDown(gfx::engine::GLContent * content)
			{
				return content->getKeyboardEvents()->isKeyDown(VK_LBUTTON) &&
					m_onDown &&
					isInside(content);
			}
			bool isHeld(gfx::engine::GLContent * content) 
			{
				return m_heldCounter > 100;
			}
			bool isReleased(gfx::engine::GLContent * content)
			{
				return content->getKeyboardEvents()->isKeyReleased(VK_LBUTTON) &&
					m_onDown;
			}
			bool isDragging(gfx::engine::GLContent * content)
			{
				return	glm::length(content->getMouseDelta()) > 0 && 
					m_onDown;
			}

			void check(gfx::engine::GLContent * content)
			{
				onPressed(isPressed(content));
				onDown(isDown(content));
				onReleased(isReleased(content));
			}

			GFXClickable() {}
			GFXClickable(GFXMesh mesh) : GFXComponent(mesh) {}

			bool m_onDown = false;
			bool m_onReleased = false;

			int m_heldCounter = 0;
		};

		class GFXButtonRect : public GFXClickable, public GFXLinker<GFXButtonRect>
		{
		public:

			void update(gfx::engine::GLContent * content)
			{
				m_back->setPos(m_pos);
				if(m_toggledState)
					m_back->setColor(glm::vec3(m_colorStyle[0]) / 2.0f);
			}
			bool checkEvents(gfx::engine::GLContent * content)
			{
				onButtonPressed(content);
				onButtonDown(content);
				onButtonReleased(content);
				onButtonDragging(content);
				return m_onDown;
			}
			void draw(gfx::engine::MeshHandle_T handles)
			{
				m_back->setPos(m_pos);
				m_back->drawMesh(handles);
				if (m_text != "")
					m_label->draw(handles);
			}
			void draw(glm::mat4 modelMat, gfx::engine::MeshHandle_T handles)
			{
				m_back->setPos(m_pos);
				m_back->drawMesh(modelMat, handles);
				if(m_text != "")
					 m_label->draw(modelMat, handles);
			}
		
			GFXComponent * init()
			{
				m_back = new GFXRectangleMesh(m_pos, m_size);
				m_back->setColor(m_colorStyle[0]);
				if (m_text != "")
				{
					m_label = new GFXLabel(m_text, GFX_GUI_DEFAULT_FONT_SIZE);
					m_label->init();
					m_label->setColor(m_colorStyle[2]);
					m_label->setPos(m_pos + m_size / 2.0f);
					m_label->center();
				}
				return this;
			}

			void validate()
			{
				m_back->setPos(m_pos);
				m_back->setSize(m_size);
				if (m_text != "")
				{
					m_label->setPos(m_pos + m_size / 2.0f);
					m_label->center();
				}
			}
			
			void onButtonPressed(gfx::engine::GLContent * content)
			{
				if (onPressed(isPressed(content)))
				{
					m_toggledState = !m_toggledState && m_isToggleable;
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
			void onButtonHeld(gfx::engine::GLContent * content)
			{
				if (isHeld(content))
				{
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
				if (isDragging(content))
				{
					callTrigger(&GFXButtonRect::onButtonDragging);
				}
			}

			GFXButtonRect(glm::vec2 pos, glm::vec2 size)
			{
				m_pos = pos;
				m_size = size;
				m_text = "";
			}
			GFXButtonRect(glm::vec2 pos, glm::vec2 size, std::string text)
			{
				m_pos = pos;
				m_size = size;
				m_text = text;
			}

			bool m_toggledState = false;
			bool m_isToggleable = false;
		protected:			
			std::string	m_text;
			GFXRectangleMesh * m_back;
			GFXLabel * m_label;
		};

		class GFXWindow : public GFXClickable, public GFXLinker<GFXWindow>
		{
		public:
			void update(gfx::engine::GLContent * content)
			{
				// get local mouse delta
				glm::vec2 delta = content->getMouseDelta();				

				// if top grab bar is being dragged
				if (m_bar->isDragging(content))
				{
					m_pos += delta;
					confinePositionToWindow(content);
					validate();
				}

				glm::vec2 pos = m_pos;
				glm::vec2 size = m_size;
				
				// if the window wants to be resized and isnt maximised
				if (m_isResizable && !m_maximised)
				{
					// horizontal grab bars resize
					if (m_isResizableX)
						if (m_leftResizeBar->isDragging(content))
						{
						
							pos.x += delta.x;
							size.x -= delta.x;
							if (!isSmallerThanMin(size, m_minSize))
							{
								m_pos = pos;
								m_size = size;
								confineSizeToContent(content);
								confinePositionToWindow(content);
								validate();
							}
						}
						else if (m_rightResizeBar->isDragging(content))
						{
							size.x += delta.x;
							if (!isSmallerThanMin(size, m_minSize))
							{
								m_size = size;
								confineSizeToContent(content);
								confinePositionToWindow(content);
								validate();
							}
						}
					// vetical grab bars resize
					if (m_isResizableY)
						if (m_bottomResizeBar->isDragging(content))
						{

							pos.y += delta.y;
							size.y -= delta.y;
							if (!isSmallerThanMin(size, m_minSize))
							{
								m_pos = pos;
								m_size = size;
								confineSizeToContent(content);
								confinePositionToWindow(content);
								validate();
							}
						}	
					// corner grabs resize
					if (m_isResizableX && m_isResizableY)
						if (m_bottomLeftResizeBar->isDragging(content))
						{
							pos.x += delta.x;
							size.x -= delta.x;
							pos.y += delta.y;
							size.y -= delta.y;
							if (!isSmallerThanMin(size, m_minSize))
							{
								m_pos = pos;
								m_size = size;
								confineSizeToContent(content);
								confinePositionToWindow(content);
								validate();
							}
						}
						else if (m_bottomRightResizeBar->isDragging(content))
						{
							size.x += delta.x;
							pos.y += delta.y;
							size.y -= delta.y;
							if (!isSmallerThanMin(size, m_minSize))
							{
								m_pos = pos;
								m_size = size;
								confineSizeToContent(content);
								confinePositionToWindow(content);
								validate();
							}
						}
				} // if m_resizeable

				// update children
				m_components.update(content);
				m_group.updateGroup(content);

			}
			bool checkEvents(gfx::engine::GLContent * content)
			{
				onWindowPressed(content);
				onWindowDown(content);
				onWindowReleased(content);
				onWindowDragging(content);
				onWindowMove(content);
				onResize(content);
				onClose(content);
				onWindowScaled(content);				
				return m_components.checkEvents(content) || m_group.checkGroupEvents(content) || m_onDown;
			}
			void draw(gfx::engine::MeshHandle_T handles)
			{				
				m_components.draw(handles);
				m_group.drawGroup(getRelativeModelMat(), handles);
			}
			void draw(glm::mat4 modelMat, gfx::engine::MeshHandle_T handles)
			{
				m_components.draw(modelMat, handles);
				m_group.drawGroup(modelMat*getRelativeModelMat(), handles);
			}

			GFXWindow * addComponent(GFXComponent * component)
			{
				m_group.add(component, m_manager->getNextId(), this, m_manager);
				validate();
				return this;
			}

			void validate()
			{
				glm::vec2 pos = m_pos, size = m_size;

				m_components.setPos(pos);
				m_components.setSize(size);

				m_leftResizeBar->setSize(glm::vec2(m_deadzone, size.y));
				m_rightResizeBar->setPos(glm::vec2(size.x - m_deadzone, 0));
				m_rightResizeBar->setSize(glm::vec2(m_deadzone, size.y));
				m_bottomResizeBar->setSize(glm::vec2(size.x, m_deadzone));
				m_bottomRightResizeBar->setPos(glm::vec2(size.x - m_deadzone, 0));

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

				m_group.validateGroup();
				m_components.validate();

				m_minSize = m_group.getMinimumBounds(GFX_GUI_DEFAULT_PADDING);
				inflateToContent();
			}

			GFXComponent * init()
			{
				glm::vec2 pos = m_pos, size = m_size;

				// window ui components container
				m_components = gfx::gui::GFXContainer(pos, size, m_colorStyle[2]);
				m_components.setAlpha(0.8f);
				m_components.setManager(m_manager);
				m_components.setColorStyle(m_colorStyle);

				// alter colorstyle for each component
				GFXColorStyle colorStyle = { glm::vec4(0,0,0,0),m_colorStyle[1],m_colorStyle[2] };

				// left resize grab
				m_leftResizeBar = new gfx::gui::GFXButtonRect(glm::vec2(), glm::vec2(m_deadzone, size.y));
				m_components.addComponent(m_leftResizeBar);
				m_leftResizeBar->setColorStyle(colorStyle);
				// right resize grab
				m_rightResizeBar = new gfx::gui::GFXButtonRect(glm::vec2(size.x - m_deadzone, 0), glm::vec2(m_deadzone, size.y));
				m_components.addComponent(m_rightResizeBar);
				m_rightResizeBar->setColorStyle(colorStyle);
				// bottom resize grab
				m_bottomResizeBar = new gfx::gui::GFXButtonRect(glm::vec2(), glm::vec2(size.x, m_deadzone));
				m_components.addComponent(m_bottomResizeBar);
				m_bottomResizeBar->setColorStyle(colorStyle);
				// bottom left resize grab
				m_bottomLeftResizeBar = new gfx::gui::GFXButtonRect(glm::vec2(), glm::vec2(m_deadzone, m_deadzone));
				m_components.addComponent(m_bottomLeftResizeBar);
				m_bottomLeftResizeBar->setColorStyle(colorStyle);
				// bottom right resize grab
				m_bottomRightResizeBar = new gfx::gui::GFXButtonRect(glm::vec2(size.x - m_deadzone, 0), glm::vec2(m_deadzone, m_deadzone));
				m_components.addComponent(m_bottomRightResizeBar);
				m_bottomRightResizeBar->setColorStyle(colorStyle);

				// top move grab
				pos.x = 0;
				pos.y = size.y - m_topBarSize;
				size.y = m_topBarSize;
				m_bar = new gfx::gui::GFXButtonRect(pos, size, m_title);
				m_components.addComponent(m_bar);

				// close window button
				pos.x = size.x - m_topBarSize;
				size.x = m_topBarSize;
				size.y = m_topBarSize;
				m_close = new gfx::gui::GFXButtonRect(pos, size);
				m_components.addComponent(m_close);
				colorStyle[0] = colorStyle[2];
				m_close->setColorStyle(colorStyle);

				// scale window button
				pos.x -= m_topBarSize;
				m_maxmin = new gfx::gui::GFXButtonRect(pos, size);
				m_components.addComponent(m_maxmin);
				colorStyle[0] = colorStyle[1];
				m_maxmin->setColorStyle(colorStyle);
				
				// initialise container
				m_components.init();

				m_group = GFXGroup();

				return this;
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
				if (isDragging(content))
				{
					callTrigger(&GFXWindow::onWindowDragging);
				}
			}
			void onWindowMove(gfx::engine::GLContent * content)
			{
				if (m_bar->isDragging(content))
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
			void onWindowScaled(gfx::engine::GLContent * content)
			{
				if (m_maxmin->isPressed(content))
				{
					toggleMaximise(content);
					callTrigger(&GFXWindow::onWindowScaled);
				}
			}

			void onResize(gfx::engine::GLContent * content)
			{
				if(m_isResizable && !m_maximised)
					if (m_leftResizeBar->isDragging(content) || m_rightResizeBar->isDragging(content) || m_bottomResizeBar->isDragging(content))
					{
						callTrigger(&GFXWindow::onResize);
					}
			}
			
			void setResizeable(bool resizable)
			{
				m_isResizable = resizable;
			}
			void setResizableVeritcal(bool resizeable)
			{
				m_isResizableX = !resizeable;
				m_isResizableY = resizeable;
			}
			void setResizableHorizontal(bool resizeable)
			{
				m_isResizableX = resizeable;
				m_isResizableY = !resizeable;
			}
			void setResizableBoth(bool resizeable)
			{
				m_isResizableX = resizeable;
				m_isResizableY = resizeable;
			}
 

			GFXWindow(glm::vec2 pos, glm::vec2 size)
			{
				m_size = size;
				m_pos = pos;
				m_minSize = glm::vec2();
				m_title = "Untitled Window";
			}
		protected:
			bool isSmallerThanMin(glm::vec2 a, glm::vec2 b)
			{
				return a.x < b.x || a.y < b.y;
			}
			void confineSizeToContent(gfx::engine::GLContent * content)
			{
				if (m_pos.x + m_size.x > content->getWindowSize().x)
					m_size.x += content->getWindowSize().x - (m_pos.x + m_size.x);
				if (m_pos.x < 0)
					m_size.x += m_pos.x;
				if (m_pos.y < 0)
					m_size.y += m_pos.y;
			}
			void confinePositionToWindow(gfx::engine::GLContent * content)
			{
				m_pos.x = min(content->getWindowSize().x - m_size.x, m_pos.x);
				m_pos.x = max(0, m_pos.x);
				m_pos.y = min(content->getWindowSize().y - m_size.y, m_pos.y);
				m_pos.y = max(0, m_pos.y);				
			}
			void inflateToContent()
			{
				m_size.x = max(m_minSize.x, m_size.x);
				m_size.y = max(m_minSize.y, m_size.y);
			}

			bool m_isResizable = true;
			bool m_isResizableX = m_isResizable;
			bool m_isResizableY = m_isResizable;

			bool m_windowMoving = false;
			bool m_maximised = false;
		
			float m_deadzone = 10;
			float m_topBarSize = 20;
			glm::vec2 m_oldPos;
			glm::vec2 m_oldSize;
			glm::vec2 m_minSize;;
			GFXButtonRect * m_bar;
			GFXButtonRect * m_leftResizeBar;
			GFXButtonRect * m_rightResizeBar;
			GFXButtonRect * m_bottomResizeBar;
			GFXButtonRect * m_bottomLeftResizeBar;
			GFXButtonRect * m_bottomRightResizeBar;
			GFXButtonRect * m_close;
			GFXButtonRect * m_maxmin;
			std::string m_title;
			GFXGroup m_group;
			GFXContainer m_components;
		};

		class GFXSpinner : public GFXContainer, public GFXLinker<GFXSpinner>
		{
		public:
			GFXComponent * init()
			{
				glm::vec2 pos = m_pos, size = m_size;			

				m_label = new GFXLabel(std::to_string(m_value), GFX_GUI_DEFAULT_FONT_SIZE);
				m_label->init();
				m_label->setColor(m_colorStyle[1]);
				m_label->setPos(m_size / 2.0f);
				m_label->center();
				addComponent(m_label);

				pos = glm::vec2();

				size.x /= 3;
				m_plus = new gfx::gui::GFXButtonRect(pos, size, "+");
				addComponent(m_plus);

				pos.x += size.x*2;
				m_minus = new gfx::gui::GFXButtonRect(pos, size, "-");
				addComponent(m_minus);				

				initGroup();

				return this;
			}
			void validate()
			{
				glm::vec2 pos = m_pos, size = m_size;

				pos = glm::vec2();

				size.x /= 4;
				m_plus->setPos(pos);
				m_plus->setSize(size);

				pos.x += size.x*3;
				m_minus->setPos(pos);
				m_minus->setSize(size);

				m_label->setPos(m_size / 2.0f);
				m_label->center();

				validateGroup();
			}
			void update(gfx::engine::GLContent * content)
			{
				updateGroup(content);
			}
			bool checkEvents(gfx::engine::GLContent * content)
			{
				onIncrease(content);
				onDecrease(content);
				return checkGroupEvents(content);
			}
			void draw(gfx::engine::MeshHandle_T handles)
			{
				drawGroup(getRelativeModelMat(), handles);
			}
			void draw(glm::mat4 modelMat, gfx::engine::MeshHandle_T handles)
			{
				drawGroup(modelMat * getRelativeModelMat(), handles);
			}

			void onIncrease(gfx::engine::GLContent * content)
			{
				if (m_plus->isReleased(content) || m_plus->isHeld(content))
				{
					setValue(++m_value);
					validate();
					callTrigger(&GFXSpinner::onIncrease);
				}
			}
			void onDecrease(gfx::engine::GLContent * content)
			{
				if (m_minus->isReleased(content) || m_minus->isHeld(content))
				{
					setValue(--m_value);
					validate();
					callTrigger(&GFXSpinner::onDecrease);
				}
			}
			
			GFXSpinner()
			{
				m_value = 0;
				m_pos = glm::vec2();
				m_size = glm::vec2();
			}
			GFXSpinner(glm::vec2 pos, glm::vec2 size, int value)
			{
				m_pos = pos;
				m_size = size;
				m_value = value;
			}
		protected:
			void setValue(int amount)
			{
				m_value = amount;
				m_label->setText(std::to_string(amount));
			}

			int m_value;
			GFXButtonRect * m_plus;
			GFXButtonRect * m_minus;
			GFXLabel * m_label;
		};

/*
		// very broken
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
*/
	}
}