
#include <thread>
#include <mutex>
#include <vector>
#include <deque>

#include <windows.h>

#include <iostream>

using namespace std;

mutex mtx;

struct RocketPosition {
public:
    int x;
    int y;
    int direction;
};

class Game {
public:
    void init(int mapWidth, int mapHeight) {
        this->mapWidth = mapWidth;
        this->mapHeight = mapHeight;
        for (int i = 0; i < mapHeight; i++) {
            vector<char> row;
            for (int j = 0; j < mapWidth; j++) {
                row.push_back('-');
            }
            map.push_back(row);
        }
    }

    void printMap() {
        Sleep(250);
        system("CLS");
        for (int i = 0; i < mapHeight; i++) {
            for (int j = 0; j < mapWidth; j++) {
                cout << "|" << map[i][j];
            }
            cout << "|" << endl;
        }
        cout << "///////////////////////////////////" << endl;
    }

    void restart() {
        // reset map to empty
        for (int i = 0; i < mapHeight; i++) {
            for (int j = 0; j < mapWidth; j++) {
                map[i][j] = '-';
            }
        }

        // reset position for all players
        hx = (mapWidth - 1) / 2;
        hy = mapHeight / 2;
        tx = (mapWidth - 1) / 2;
        ty = 0;
        rx = mapWidth - 1;
        ry = mapHeight / 2;

        // reset can shoot
        for (int i = 0; i < 10; i++) {
            canShootH[i] = true;
        }
    }

    void updateMap() {
        // update map with all player position
        map[hy][hx] = 'H';
        map[ty][tx] = 'T';
        map[ry][rx] = 'R';

        // update map with all rockets
        for (int i = 0; i < hRockets.size(); i++) { // H rockets
            map[hRockets[i].y][hRockets[i].x] = '#';
        }

        for (int i = 0; i < tRockets.size(); i++) { // T rockets
            map[tRockets[i].y][tRockets[i].x] = '*';
        }

        for (int i = 0; i < rRockets.size(); i++) { // R rockets
            if(rRockets[i].direction == 7)
                map[rRockets[i].y][rRockets[i].x] = 'M';
            if (rRockets[i].direction == 1)
                map[rRockets[i].y][rRockets[i].x] = 'W';
        }
    }

    void updateMoveRocket() {
        for (int i = 0; i < hRockets.size(); i++) { // H rockets
            map[hRockets[i].y][hRockets[i].x] = '-';
            switch (hRockets[i].direction) {
            case 8:
                hRockets[i].y--;
                break;
            case 2:
                hRockets[i].y++;
                break;
            case 4:
                hRockets[i].x--;
                break;
            case 6:
                hRockets[i].x++;
                break;
            case 7:
                hRockets[i].x--;
                hRockets[i].y--;
                break;
            case 9:
                hRockets[i].x++;
                hRockets[i].y--;
                break;
            case 1:
                hRockets[i].x--;
                hRockets[i].y++;
                break;
            case 3:
                hRockets[i].x++;
                hRockets[i].y++;
                break;
            }
        }

        for (int i = 0; i < tRockets.size(); i++) { // T rockets
            map[tRockets[i].y][tRockets[i].x] = '-';
            tRockets[i].y++;
        }

        for (int i = 0; i < rRockets.size(); i++) { // R rockets
            map[rRockets[i].y][rRockets[i].x] = '-';
            if (rRockets[i].direction == 7) {
                rRockets[i].x--;
                rRockets[i].y--;
            }
            if (rRockets[i].direction == 1) {
                rRockets[i].x--;
                rRockets[i].y++;
            }
        }
    }

    void removeOutofMapRockets() {
        deque<RocketPosition>::iterator i = hRockets.begin();
        deque<deque<RocketPosition>::iterator> iToDelete;

        while (i != hRockets.end()) {
            if (i->x < 0 || i->x >= mapWidth || i->y < 0 || i->y >= mapHeight) { // H Rocket
                iToDelete.push_back(i);
            }
            i++;
        }
        for (int j = 0; j < iToDelete.size(); j++) {
            canShootH[iToDelete[j]->direction] = true;
            hRockets.erase(iToDelete[j]);
        }

        iToDelete.clear();
        i = tRockets.begin();
        while (i != tRockets.end()) {
            if (i->y >= mapHeight) { // T Rocket
                iToDelete.push_back(i);
            }
            i++;
        }
        for (int j = 0; j < iToDelete.size(); j++) {
            tRockets.erase(iToDelete[j]);
        }

        iToDelete.clear();
        i = rRockets.begin();
        while (i != rRockets.end()) {
            if (i->x < 0 || i->y < 0 || i->y >= mapHeight) { // R Rocket
                iToDelete.push_back(i);
            }
            i++;
        }
        for (int j = 0; j < iToDelete.size(); j++) {
            rRockets.erase(iToDelete[j]);
        }
    }

    void hMove(char direction) {
        if (canMoveH(direction)) {
            removePreviousH();

            switch (direction) {
            case '8':
                hy--;
                break;
            case '2':
                hy++;
                break;
            case '4':
                hx--;
            case '6':
                hx++;
                break;
            case '7':
                hx--;
                hy--;
                break;
            case '9':
                hx++;
                hy--;
                break;
            case '1':
                hx--;
                hy++;
                break;
            case '3':
                hx++;
                hy++;
                break;
            }
        }
        updateMap();
        printMap();
    }

    void tMove(char direction) {
        if (canMoveT(direction)) {
            removePreviousT();
            switch (direction) {
            case '4':
                tx--;
                break;
            case '6':
                tx++;
                break;
            }
        }
        updateMap();
        printMap();
    }

    void rMove(char direction) {
        removePreviousR();
        if (canMoveR(direction)) {
            switch (direction) {
            case '8':
                ry--;
                break;
            case '2':
                ry++;
                break;
            }
        }
        updateMap();
        printMap();
    }

    void hShoot(char direction) {
        int intDirection = charToInt(direction);
        if (canShootH[intDirection]) {
            canShootH[intDirection] = false;
            RocketPosition newRocket;
            newRocket.direction = intDirection;
            switch (direction) {
            case '8': // top
                newRocket.x = hx;
                newRocket.y = hy-1;
                break;
            case '2': // down
                newRocket.x = hx;
                newRocket.y = hy + 1;
                break;
            case '4': // left
                newRocket.x = hx - 1;
                newRocket.y = hy;
                break;
            case '6': // right
                newRocket.x = hx + 1;
                newRocket.y = hy;
                break;
            case '7': // top left
                newRocket.x = hx - 1;
                newRocket.y = hy - 1;
                break;
            case '9': // top right
                newRocket.x = hx + 1;
                newRocket.y = hy - 1;
                break;
            case '1': // down left
                newRocket.x = hx - 1;
                newRocket.y = hy + 1;
                break;
            case '3': // down right
                newRocket.x = hx + 1;
                newRocket.y = hy + 1;
                break;
            }
            hRockets.push_back(newRocket);
        }
        updateMap();
        printMap();
    }

    void tShoot() {
        if (canShootT()) {
            RocketPosition newRocket;
            newRocket.direction = 2;

            newRocket.x = tx;
            newRocket.y = ty + 1;

            tRockets.push_back(newRocket);
        }

        updateMap();
        printMap();
    }

    void rShoot() {
        RocketPosition newRocketM;
        RocketPosition newRocketW;
        newRocketM.direction = 7;
        newRocketW.direction = 1;

        if (canShootR()){
            newRocketM.x = rx - 1;
            newRocketM.y = ry - 1;

            newRocketW.x = rx - 1;
            newRocketW.y = ry + 1;

            rRockets.push_back(newRocketM);
            rRockets.push_back(newRocketW);
        }

        updateMap();
        printMap();
    }

    int charToInt(char c) {
        return ((int)c - 48);
    }

private:
    int mapWidth, mapHeight;
    vector<vector<char>> map;

    int hx, hy;
    int tx, ty;
    int rx, ry;

    deque<RocketPosition> hRockets;
    deque<RocketPosition> tRockets;
    deque<RocketPosition> rRockets;

    bool canShootH[10];

    bool canMoveH(char direction) {
        switch (direction) {
        case '8':
            if ((hy - 1) == 0)
                return false;
            break;
        case '2':
            if ((hy + 1) == (mapHeight))
                return false;
            break;
        case '4':
            if ((hx - 1) < 0)
                return false;
            break;
        case '6':
            if ((hx + 1) == (mapWidth - 1))
                return false;
            break;
        case '7':
            if ((hx - 1) < 0 && (hy - 1) == 0)
                return false;
            break;
        case '9':
            if ((hx + 1) == (mapWidth - 1) && (hy - 1) == 0)
                return false;
            break;
        case '1':
            if ((hx - 1) < 0 && (hy + 1) == mapHeight)
                return false;
            break;
        case '3':
            if ((hx + 1) == (mapWidth - 1) && (hy + 1) == mapHeight)
                return false;
            break;
        }

        return true;
    }

    bool canMoveT(char direction) {
        switch (direction) {
        case '4':
            if (tx - 1 < 0) {
                return false;
            }
            break;
        case '6':
            if (tx + 1 == (mapWidth - 1)) {
                return false;
            }
            break;
        }
        return true;
    }

    bool canMoveR(char direction) {
        switch (direction) {
        case '8':
            if (ry - 1 < 0) {
                return false;
            }
            break;
        case '2':
            if (ry + 1 == mapHeight) {
                return false;
            }
            break;
        }
        return true;
    }

    void removePreviousH() { // call first
        map[hy][hx] = '-';
    }

    void removePreviousT() { // call first
        map[ty][tx] = '-';
    }

    void removePreviousR() { // call first
        map[ry][rx] = '-';
    }

    bool canShootT() {
        for (int i = 0; i < tRockets.size(); i++) {
            if (tRockets[i].x == tx) {
                if (tRockets[i].y <= 2)
                    return false;
            }
        }
        return true;
    }

    bool canShootR() {
        for (int i = 0; i < rRockets.size(); i++) {
            if (rRockets[i].x == (rx - 2) && rRockets[i].y == (ry - 2)) {
                return false;
            }
            if (rRockets[i].x == (rx - 1) && rRockets[i].y == (ry - 1)) {
                return false;
            }
        }

        for (int i = 0; i < rRockets.size(); i++) {
            if (rRockets[i].x == (rx - 2) && rRockets[i].y == (ry + 2)) {
                return false;
            }
            if (rRockets[i].x == (rx - 1) && rRockets[i].y == (ry + 1)) {
                return false;
            }
        }
        
        return true;
    }
};

void function_thread(int n) {
    thread::id this_id = this_thread::get_id();
    for (int i = 0; i < n; i++) {
       mtx.lock();
       mtx.unlock();
    }
   
}

int main()
{
    /*
    thread thread1(function_thread, 3);
    thread thread2(function_thread, 3);
    thread thread3(function_thread, 3);

    thread1.join();
    thread2.join();
    thread3.join();
    */

    Game game;
    game.init(10,10);
    game.restart();
    game.updateMap();
    game.printMap();

    game.tMove(2);
    game.tShoot();
    game.rShoot();
    game.hShoot('1');
    game.hShoot('2');
    game.hShoot('3');
    game.hShoot('4');
    game.hShoot('6');
    game.hShoot('7');
    game.hShoot('8');
    game.hShoot('9');

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.tMove('4');
    game.tShoot();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.rShoot();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.rMove('2');
    game.rMove('2');
    game.rShoot();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.hMove('8');
    game.hMove('9');
    game.hShoot('1');

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.hShoot('9');

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();

    game.updateMoveRocket();
    game.removeOutofMapRockets();
    game.updateMap();
    game.printMap();
}
