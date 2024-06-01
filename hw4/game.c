#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// 전역 변수 선언
int player_x, player_y;
int max_x, max_y;
int game_running = 1;

// 시그널 핸들러
void handle_signal(int sig) {
    game_running = 0;
}

void init_game() {
    initscr(); // ncurses 초기화
    cbreak(); // 줄 버퍼링 없이 문자 단위 입력
    noecho(); // 입력된 문자를 화면에 표시하지 않음
    keypad(stdscr, TRUE); // 특수 키 입력 허용
    curs_set(0); // 커서 감춤

    // 화면 크기 가져오기
    getmaxyx(stdscr, max_y, max_x);

    // 플레이어 초기 위치 설정
    player_x = max_x / 2;
    player_y = max_y / 2;

    // 시그널 핸들러 설정
    signal(SIGINT, handle_signal);
}

void end_game() {
    endwin(); // ncurses 종료
}

void draw_player() {
    mvprintw(player_y, player_x, "P"); // 플레이어 그리기
    refresh(); // 화면 갱신
}

void update_position(int ch) {
    // 이전 위치 지우기
    mvprintw(player_y, player_x, " ");

    switch (ch) {
        case KEY_UP:
            player_y = (player_y > 0) ? player_y - 1 : player_y;
            break;
        case KEY_DOWN:
            player_y = (player_y < max_y - 1) ? player_y + 1 : player_y;
            break;
        case KEY_LEFT:
            player_x = (player_x > 0) ? player_x - 1 : player_x;
            break;
        case KEY_RIGHT:
            player_x = (player_x < max_x - 1) ? player_x + 1 : player_x;
            break;
    }
}

int main() {
    int ch;

    init_game();

    while (game_running) {
        clear(); // 화면 지우기
        draw_player(); // 플레이어 그리기
        ch = getch(); // 키 입력 받기
        update_position(ch); // 위치 업데이트
    }

    end_game();
    return 0;
}
