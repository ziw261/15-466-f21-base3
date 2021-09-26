#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>


#define PLAYER_SPEED 100.0f

struct GardenMode : Mode {

	struct Player {
		Scene::Transform* transform = nullptr;
		Player() {}
		Player(Scene::Transform* trans) : transform(trans) {}
	};

	GardenMode();
	virtual ~GardenMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Player player;
	glm::quat default_rot;

	//glm::vec3 get_leg_tip_position();

	void LoadGameObjects();
	void UpdatePlayerMovement(float elapsed);

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
