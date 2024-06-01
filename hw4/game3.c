#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define MAX_STARS 50
#define SHIP_CHAR 'A'
#define STAR_CHAR '*'

typedef struct {
    int x, y;
} Position;

Position stars[MAX_STARS];
int num_stars = MAX_STARS;
Position ship;
int max_x, max_y;
int game_running = 1;

void handle_signal(int sig) {
    game_running = 0;
}

void init_game() {
    initscr(); // ncurses 초기화
    cbreak(); // 줄 버퍼링 없이 문자 단위 입력
    noecho(); // 입력된 문자를 화면에 표시하지 않음
    keypad(stdscr, TRUE); // 특수 키 입력 허용
    curs_set(0); // 커서 감춤
    nodelay(stdscr, TRUE); // 비차단 입력

    // 화면 크기 가져오기
    getmaxyx(stdscr, max_y, max_x);

    // 우주선 초기 위치 설정
    ship.x = max_x / 2;
    ship.y = max_y - 1;

    // 별의 초기 위치 설정
    srand(time(NULL));
    for (int i = 0; i < num_stars; i++) {
        stars[i].x = rand() % max_x;
        stars[i].y = rand() % max_y;
    }

    // 시그널 핸들러 설정
    signal(SIGINT, handle_signal);
}

void end_game() {
    endwin(); // ncurses 종료
}

void draw_ship() {
    mvprintw(ship.y, ship.x, "%c", SHIP_CHAR);
}

void draw_stars() {
    for (int i = 0; i < num_stars; i++) {
        mvprintw(stars[i].y, stars[i].x, "%c", STAR_CHAR);
    }
}

void update_stars() {
    for (int i = 0; i < num_stars; i++) {
        mvprintw(stars[i].y, stars[i].x, " "); // 별 지우기
        stars[i].y += 1;
        if (stars[i].y >= max_y) {
            stars[i].y = 0;
            stars[i].x = rand() % max_x;
        }
    }
}

void update_ship(int ch) {
    mvprintw(ship.y, ship.x, " "); // 이전 위치 지우기

    switch (ch) {
        case KEY_LEFT:
            ship.x = (ship.x > 0) ? ship.x - 1 : ship.x;
            break;
        case KEY_RIGHT:
            ship.x = (ship.x < max_x - 1) ? ship.x + 1 : ship.x;
            break;
    }
}

int check_collision() {
    for (int i = 0; i < num_stars; i++) {
        if (stars[i].y == ship.y && stars[i].x == ship.x) {
            return 1;
        }
    }
    return 0;
}

int main() {
    int ch;

    init_game();

    while (game_running) {
        clear(); // 화면 지우기
        draw_ship(); // 우주선 그리기
        draw_stars(); // 별 그리기
        refresh(); // 화면 갱신

        ch = getch(); // 키 입력 받기
        update_ship(ch); // 우주선 위치 업데이트
        update_stars(); // 별 위치 업데이트

        if (check_collision()) {
            game_running = 0;
            mvprintw(max_y / 2, max_x / 2 - 5, "GAME OVER");
            refresh();
            sleep(2);
        }

        usleep(200000); // 게임 속도 조절
    }

    end_game();
    return 0;
}
