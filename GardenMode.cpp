#include "GardenMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint garden_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("garden.pnct"));
	garden_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("garden.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = garden_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

GardenMode::GardenMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	//for (auto &transform : scene.transforms) {
	//	if (transform.name == "Hip.FL") hip = &transform;
	//	else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
	//	else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
	//}
	//if (hip == nullptr) throw std::runtime_error("Hip not found.");
	//if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	//if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

	//hip_base_rotation = hip->rotation;
	//upper_leg_base_rotation = upper_leg->rotation;
	//lower_leg_base_rotation = lower_leg->rotation;

	LoadGameObjects();

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
	//leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
}

GardenMode::~GardenMode() {
}

void GardenMode::LoadGameObjects() {
	for (auto& transform : scene.transforms) {
		if (transform.name == "opossum") {
			player = Player(&transform);
			default_rot = player.transform->rotation;
		}
		if (transform.name == "dirt") {
			glm::vec3 pos = transform.position;
			walls[0] = pos.x - 150.f;
			walls[1] = pos.x + 150.f;
			walls[2] = pos.y - 100.f;
			walls[3] = pos.y + 100.f;
		}
	}

	assert(player.transform != nullptr);
}

bool GardenMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void GardenMode::update(float elapsed) {

	UpdatePlayerMovement(elapsed);

	//move sound to follow leg tip position:
	//leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);

	//move camera:
	//{

	//	//combine inputs into a move:
	//	constexpr float PlayerSpeed = 30.0f;
	//	glm::vec2 move = glm::vec2(0.0f);
	//	if (left.pressed && !right.pressed) move.x =-1.0f;
	//	if (!left.pressed && right.pressed) move.x = 1.0f;
	//	if (down.pressed && !up.pressed) move.y =-1.0f;
	//	if (!down.pressed && up.pressed) move.y = 1.0f;

	//	//make it so that moving diagonally doesn't go faster:
	//	if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

	//	glm::mat4x3 frame = camera->transform->make_local_to_parent();
	//	glm::vec3 right = frame[0];
	//	//glm::vec3 up = frame[1];
	//	glm::vec3 forward = -frame[2];

	//	camera->transform->position += move.x * right + move.y * forward;
	//}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}
}

void GardenMode::UpdatePlayerMovement(float elapsed) {
	glm::vec2 player_move = glm::vec2(0.0f);
	if (up.pressed) player_move.y += 1.0f;
	if (left.pressed) player_move.x -= 1.0f;
	if (down.pressed) player_move.y -= 1.0f;
	if (right.pressed) player_move.x += 1.0f;

	if (player_move != glm::vec2(0.0f)) {
		player_move = glm::normalize(player_move) * PLAYER_SPEED * elapsed;

		// Player rotation with movement
		if (player_move.x && player_move.y) {
			float rot_val = ((player_move.x < 0) - (player_move.x > 0)) * (90.f * (player_move.y < 0) + 45);
			player.transform->rotation = glm::angleAxis(glm::radians(rot_val), glm::vec3(0.f, 0.f, 1.f)) * default_rot;
		}
		else if (!player_move.y)
			player.transform->rotation = glm::angleAxis(glm::radians(((player_move.x < 0) - (player_move.x > 0)) * 90.f), glm::vec3(0.f, 0.f, 1.f)) * default_rot;
		else if (!player_move.x)
			player.transform->rotation = glm::angleAxis(glm::radians((player_move.y < 0) * 180.f), glm::vec3(0.f, 0.f, 1.f)) * default_rot;
	}

	glm::vec3 movement = glm::vec3(player_move.x, player_move.y, 0);
	player.transform->position += movement;

	up.downs = 0;
	down.downs = 0;
	left.downs = 0;
	right.downs = 0;

	//clamp player position value:
	auto& pos = player.transform->position;
	/*std::cout << walls[0] << " " <<
		walls[1] << " " <<
		walls[2] << " " <<
		walls[3] << " " << std::endl;
	std::cout << pos.x << " " << pos.y << std::endl;*/
	pos.x = std::max(pos.x, walls[0] + player.size.x * 0.5f);
	pos.x = std::min(pos.x, walls[1] - player.size.x * 0.5f);
	pos.y = std::max(pos.y, walls[2] + player.size.x * 0.5f);
	pos.y = std::min(pos.y, walls[3] - player.size.x * 0.5f);
}

void GardenMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

//glm::vec3 GardenMode::get_leg_tip_position() {
//	//the vertex position here was read from the model in blender:
//	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
//}
