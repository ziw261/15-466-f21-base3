#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>


#define PLAYER_SPEED 100.f
#define CABBAGE_SIZE glm::vec3(35.f, 31.f, 41.f)
#define CARROT_SIZE glm::vec3(5.f, 50.f, 7.f)
#define CABBAGE_EATTIME 3.5f
#define CARROT_EATTIME 1.5f
#define HIDE_SPEED 20.f
#define FOOTSTEP_START -270.f
#define FOOTSTEP_SPEED 20.f
#define FOOTSTEP_MINSPAWNTIME 15.f
#define FOOTSTEP_SPAWNDIFF 10


struct GardenMode : Mode {

	struct Player {
		Scene::Transform* transform = nullptr;
		glm::vec3 size = glm::vec3(20.f, 10.f, 20.f);
		Player() {}
		Player(Scene::Transform* trans) : transform(trans) {}
	};

	struct Food {
		Scene::Transform* transform = nullptr;
		glm::vec3 size = glm::vec3(30.f, 31.f, 41.f);
		float lifetime = 0.f;
		Food() {}
		Food(Scene::Transform* trans, glm::vec3 food_size, float t) : transform(trans), size(food_size), lifetime(t) {}
	};

	enum class TextStatus {
		Default,
		Eating,
		Hiding,
		Hidden,
		Lose,
		Win
	};

	enum class AudioStatus {
		None,
		Footsteps
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
	} left, right, down, up, eat, hide;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Player player;
	glm::quat default_rot;
	int target = -1;
	float walls[4];
	std::string show_text = "";
	bool is_hiding = false;
	bool is_hidden = false;
	bool has_spawned = false;
	bool begin_check = false;
	bool is_game_over = false;

	std::vector<Food> foods;
	glm::vec3 footsteps_pos = glm::vec3(walls[0] + FOOTSTEP_START, 30.f, 16.5f);

	void LoadGameObjects();
	void UpdatePlayerMovement(float elapsed);
	void UpdateEating(float elapsed);
	void UpdateHiding(float elapsed);
	void UpdateShowText(float elapsed, TextStatus ts);
	void UpdateFootSteps(float elapsed);
	void UpdateAudio();
	void PlayAudio(AudioStatus as, bool to_start);
	bool CollisionTest(glm::vec2 pos);
	void UpdateGameStatus(float elapsed);
	glm::vec3 get_foot_position();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > footsteps;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
