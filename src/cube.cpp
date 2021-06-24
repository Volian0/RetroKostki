#include "cube.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "player.h"

#include <C:\LearnOpenGL-master\includes\learnopengl\shader_m.h>

extern Player player;

void render_cube(glm::vec3 pos, Texture* texture, Shader* shader, double delta)
{
	texture->use();
	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	model = glm::translate(model, pos);
	if (delta)
	{
		model = glm::scale(model, glm::vec3(1.0 - (delta*delta*delta*delta*delta*0.25)));
	}
	shader->setMat4("model", model);

	glDrawArrays(GL_TRIANGLES, 0, 36);
}
