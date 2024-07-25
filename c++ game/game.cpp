#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player1_pos, player1_velocity, player2_pos, player2_velocity;

float arena_half_size_x = 80, arena_half_size_y = 45;

float player_half_size_x = 2.5, player_half_size_y = 12;

float ball_pos_x, ball_pos_y, ball_velocity_x = 130, ball_velocity_y, ball_half_size = 1;

int player1_score, player2_score;

static void max_player_height(float *p, float *v, float a, float dt) {

    a -= *v * 10.f;

    *p = *p + *v * dt + a * dt * dt * .5f;
    *v = *v + a * dt;

    if (*p + player_half_size_y > arena_half_size_y) {
        *p = arena_half_size_y - player_half_size_y;
        *v = 0;
    }
    else if (*p - player_half_size_y < -arena_half_size_y) {
        *p = -arena_half_size_y + player_half_size_y;
        *v = 0;
    }
}

static bool ball_collision(float ball_pos_x, float ball_pos_y, float ball_half_size, float players_half_size,
    float ball_half_size2, float player_pos, float player_half_size_x, float player_half_size_y) {
    return (ball_pos_x + ball_half_size > ball_half_size2 - player_half_size_x &&
        ball_pos_x - ball_half_size < ball_half_size2 + player_half_size_x &&
        ball_pos_y + players_half_size > player_pos - player_half_size_y &&
        ball_pos_y + players_half_size < player_pos + player_half_size_y);
}

enum Gamemode {
    GM_MENU,
    GM_GAMEPLAY,
};

Gamemode current_gamemode;
int hot_button;
bool ai_enemy;

static void simulate_game(Input* input, float dt) {

    draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0xffaa33);
    draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xff5500);

    if (current_gamemode == GM_GAMEPLAY) {
        float player1_accelaration = 0.f;
        if (!ai_enemy) {
            if (is_down(BUTTON_UP)) {
                player1_accelaration += 1600;
            }
            if (is_down(BUTTON_DOWN)) {
                player1_accelaration -= 1600;
            }
        }
        else {
            player1_accelaration = (ball_pos_y - player1_pos) * 20;
            if (player1_accelaration > 1000) player1_accelaration = 1000;
            if (player1_accelaration < -1000) player1_accelaration = -1000;
        }

        float player2_accelaration = 0.f;

        if (is_down(BUTTON_W)) {
            player2_accelaration += 1600;
        }
        if (is_down(BUTTON_S)) {
            player2_accelaration -= 1600;
        }

        max_player_height(&player1_pos, &player1_velocity, player1_accelaration, dt);
        max_player_height(&player2_pos, &player2_velocity, player2_accelaration, dt);

        player1_accelaration -= player1_velocity * 10.f; //Friction
        player1_pos = player1_pos + player1_velocity * dt + player1_accelaration * dt * dt * .5f;
        player1_velocity = player1_velocity + player1_accelaration * dt;

        player2_accelaration -= player2_velocity * 10.f;
        player2_pos = player2_pos + player2_velocity * dt + player2_accelaration * dt * dt * .5f;
        player2_velocity = player2_velocity + player2_accelaration * dt;

        ball_pos_x += ball_velocity_x * dt;
        ball_pos_y += ball_velocity_y * dt;

        if (ball_collision(ball_pos_x, ball_pos_y, ball_half_size, ball_half_size, 80, player1_pos, player_half_size_x, player_half_size_y)) {
            ball_pos_x = 80 - player_half_size_x - ball_half_size;
            ball_velocity_x *= -1;
            ball_velocity_y = (ball_pos_y - player1_pos) * 2 + player1_velocity * .75f;
        }
        else if (ball_collision(ball_pos_x, ball_pos_y, ball_half_size, ball_half_size, -80, player2_pos, player_half_size_x, player_half_size_y)) {
            ball_pos_x = -80 + player_half_size_x + ball_half_size;
            ball_velocity_x *= -1;
            ball_velocity_y = (ball_pos_y - player2_pos) * 2 + player2_velocity * .75f;
        }

        if (ball_pos_y + ball_half_size > arena_half_size_y) {
            ball_pos_y = arena_half_size_y - ball_half_size;
            ball_velocity_y *= -1;
        }
        else if (ball_pos_y + ball_half_size < -arena_half_size_y) {
            ball_pos_y = -arena_half_size_y + ball_half_size;
            ball_velocity_y *= -1;
        }

        if (ball_pos_x + ball_half_size > arena_half_size_x) {
            ball_velocity_x *= -1;
            ball_velocity_y = 0;
            ball_pos_x = 0;
            ball_pos_y = 0;
            ++player1_score;
        }
        else if (ball_pos_x - ball_half_size < -arena_half_size_x) {
            ball_velocity_x *= -1;
            ball_velocity_y = 0;
            ball_pos_x = 0;
            ball_pos_y = 0;
            ++player2_score;
        }

        draw_score(player1_score, -10, 40, 1.f, 0xbbffbb);
        draw_score(player2_score, 10, 40, 1.f, 0xbbffbb);

        draw_rect(ball_pos_x, ball_pos_y, ball_half_size, ball_half_size, 0x00ffff);

        draw_rect(80, player1_pos, player_half_size_x, player_half_size_y, 0xff0000);
        draw_rect(-80, player2_pos, player_half_size_x, player_half_size_y, 0xff0000);
    }
    else {

        if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
            hot_button = !hot_button;
        }
        if (pressed(BUTTON_ENTER)) {
            current_gamemode = GM_GAMEPLAY;
            ai_enemy = hot_button ? 0 : 1;
        }
        if (hot_button == 0) {
            draw_text("SINGLE PLAYER", -80, -10, 1, 0xff0000);
            draw_text("MULTIPLAYER", 20, -10, 1, 0xaaaaaa);
        }
        else {
            draw_text("SINGLE PLAYER", -80, -10, 1, 0xaaaaaa);
            draw_text("MULTIPLAYER", 20, -10, 1, 0xff0000);
        }

        draw_text("PONG GAME", -73, 40, 2, 0xffffff);
        draw_text("GAME DEV PROJECT", -73, 22, .75, 0xffffff);
        draw_text("SHREYASH HAMAL", -73, 15, 1.22, 0xffffff);
        
    }
}