        
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

        reset();
    }

    void printMap() {
        system("CLS");
        for (int i = 0; i < mapHeight; i++) {
            for (int j = 0; j < mapWidth; j++) {
                cout << "|" << map[i][j];
            }
            cout << "|" << endl;
        }
        cout << "///////////////////////////////////" << endl;
        cout << "Home Score : " << homeScore << " Guest Score: " << guestScore << " Number of Restart: " << numberOfRound  << endl;
        cout << "///////////////////////////////////" << endl;
    }

    void reset() { 
        homeScore = 0;
        guestScore = 0;
        numberOfRound = 0;
        gameOver = false;

        restart();
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

        restartGame = false;
        disableR = false;
        disableT = false;

        hRockets.clear();
        tRockets.clear();
        rRockets.clear();
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
            if (i->x <= 0 || i->y <= 0 || i->y >= mapHeight) { // R Rocket
                iToDelete.push_back(i);
            }
            i++;
        }
        for (int j = 0; j < iToDelete.size(); j++) {
            rRockets.erase(iToDelete[j]);
        }

        collisionDetection(); 
        checkRestartGame();
    }

    void collisionDetection() {
        deque<int> indexHToDelete, indexTToDelete, indexRToDelete;
        for (int i = 0; i < tRockets.size(); i++) {
            if (tRockets[i].x == hx && tRockets[i].y == hy) { // check t and r rockets hit H, t and r score 20 points, restart game
                // +20 score to guest
                guestScore += 20;
                restartGame = true;
                return;
            }
            for (int j = 0; j < hRockets.size(); j++) {
                if (tRockets[i].x == hRockets[j].x && tRockets[i].y == hRockets[j].y) { // check h rockets hit rockets from t and r, h get 2 points
                    // +2 score to home
                    homeScore += 2;
                    indexHToDelete.push_back(j);// cout << "--------------------dsdsdsds----" << j << endl; Sleep(1000);
                    indexTToDelete.push_back(i);
                }
            }
        }
        for (int i = 0; i < rRockets.size(); i++) {
            if (rRockets[i].x == hx && rRockets[i].y == hy) { // check t and r rockets hit H, t and r score 20 points, restart game
                // +20 score to guest
                guestScore += 20;
                restartGame = true;
                return;
            }
            for (int j = 0; j < hRockets.size(); j++) {
                if (rRockets[i].x == hRockets[j].x && rRockets[i].y == hRockets[j].y) { // check h rockets hit rockets from t and r, h get 2 points
                    // +2 score to home
                    homeScore += 2;
                    indexHToDelete.push_back(j);// cout << "--------------------dsdsdsds----" << j << endl; Sleep(1000);
                    indexRToDelete.push_back(i);
                }
            }
        } 

        // delete rocket
        deleteAllHRocketsByIndex(indexHToDelete);
        deleteAllTRocketsByIndex(indexTToDelete);
        deleteAllRRocketsByIndex(indexRToDelete);


        // check h rockets hit t and r, h get 4 points, disable t and r when hit by h
        for (int i = 0; i < hRockets.size(); i++) {
            if (hRockets[i].x == tx && hRockets[i].y == ty) {
                // +4 score to home
                homeScore += 4;
            }
            if (hRockets[i].x == rx && hRockets[i].y == ry) {
                // +4 score to home
                homeScore += 4;
            }
        }

        // check if both t and r is disabled, if yes then restart 
        if (disableT && disableR)
            restartGame = true;
    }

    void checkRestartGame() {
        // check if game restart
        if (restartGame) {
            if (numberOfRound == 4) {
                gameOver = true;
            }

            numberOfRound++;
            restart();
            updateMap();
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
                break;
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
            
        //updateMap();
        //printMap();
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
        //updateMap();
        //printMap();
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
        //updateMap();
        //printMap();
    }

    void hShoot(char direction) {
        if (hx != 0 && hy != (mapHeight - 1)) {
            int intDirection = charToInt(direction);
            if (canShootH[intDirection]) {
                canShootH[intDirection] = false;
                RocketPosition newRocket;
                newRocket.direction = intDirection;
                switch (direction) {
                case '8': // top
                    newRocket.x = hx;
                    newRocket.y = hy - 1;
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
        }


       
        //updateMap();
        //printMap();
    }

    void tShoot() {
        if (canShootT()) {
            RocketPosition newRocket;
            newRocket.direction = 2;

            newRocket.x = tx;
            newRocket.y = ty + 1;

            tRockets.push_back(newRocket);
        }

        //updateMap();
        //printMap();
    }

    void rShoot() {
        RocketPosition newRocketM;
        RocketPosition newRocketW;
        newRocketM.direction = 7;
        newRocketW.direction = 1;

        if (canShootR()){
            newRocketM.x = rx - 1;
            newRocketM.y = ry - 1;
            rRockets.push_back(newRocketM);

            if (ry < (mapHeight - 1)) {
                newRocketW.x = rx - 1;
                newRocketW.y = ry + 1;
                rRockets.push_back(newRocketW);
            }
        }

        //updateMap();
        //printMap();
    }

    int charToInt(char c) {
        return ((int)c - 48);
    }

    bool getGameOver() {
        return gameOver;
    }

    void printWinner() {
        cout << "===== Game Over =====" << endl;
        if (homeScore > guestScore)
            cout << "The winner is Home! Well Done H!" << endl;
        else
            cout << "The winner is Guest! Well Done T and R!" << endl;
        cout << "=====================" << endl;
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

    bool disableT, disableR, restartGame;

    int homeScore, guestScore;

    int numberOfRound;

    bool gameOver;

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
            if ((hx - 1) < 0 || (hy - 1) == 0)
                return false;
            break;
        case '9':
            if ((hx + 1) == (mapWidth - 1) || (hy - 1) == 0)
                return false;
            break;
        case '1':
            if ((hx - 1) < 0 || (hy + 1) == mapHeight)
                return false;
            break;
        case '3':
            if ((hx + 1) == (mapWidth - 1) || (hy + 1) == mapHeight)
                return false;
            break;
        }

        return true;
    }

    bool canMoveT(char direction) {
        if (disableR) {
            return false;
        }

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
        if (disableT) {
            return false;
        }

        switch (direction) {
        case '8':
            if (ry == 1) {
                return false;
            }
            break;
        case '2':
            if (ry == (mapHeight - 1)) {
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
        if (disableT) {
            return false;
        }

        for (int i = 0; i < tRockets.size(); i++) {
            if (tRockets[i].x == tx) {
                if (tRockets[i].y <= 2)
                    return false;
            }
        }
        return true;
    }

    bool canShootR() {
        if (disableR) {
            return false;
        }

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

    void deleteAllHRocketsByIndex(deque<int> indexHToDelete) {
        deque<RocketPosition> newHRockets;
        for (int i = 0; i < hRockets.size(); i++) {
            if (!iExistIn(i, indexHToDelete)) {
                newHRockets.push_back(hRockets[i]);
            }
            else {
                canShootH[hRockets[i].direction];
            }
        }
        hRockets = newHRockets;
    }

    void deleteAllTRocketsByIndex(deque<int> indexTToDelete) {
        deque<RocketPosition> newTRockets;
        for (int i = 0; i < tRockets.size(); i++) {
            if (!iExistIn(i, indexTToDelete)) {
                newTRockets.push_back(tRockets[i]);
            }
        }
        tRockets = newTRockets;
    }

    void deleteAllRRocketsByIndex(deque<int> indexRToDelete) {
        deque<RocketPosition> newRRockets;
        for (int i = 0; i < rRockets.size(); i++) {
            if (!iExistIn(i, indexRToDelete)) {
                newRRockets.push_back(rRockets[i]);
            }
        }
        rRockets = newRRockets;
    }

    bool iExistIn(int target, deque<int> indexHToDelete) {
        for (int i = 0; i < indexHToDelete.size(); i++)
            if (target == indexHToDelete[i])
                return true;
        
        return false;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void hThread_Function(Game &game) {
    thread::id this_id = this_thread::get_id();
    
    
    char moveDirectionChoice, shootDirectionChoice, moveOrShootChoice;
    srand(time(NULL));
    char possibleChoice[8] = { '1','2','3','4','6','7','8','9' };
    char possibleMoveorShoot[2] = { 'm', 's' };
    do {
        // decide to move or shoot
        moveOrShootChoice = possibleMoveorShoot[rand() % 2];
        moveDirectionChoice = possibleChoice[rand() % 8];
        shootDirectionChoice = moveDirectionChoice;

        mtx.lock();
            if (moveOrShootChoice == 'm')
                game.hMove(moveDirectionChoice);
            else if (moveOrShootChoice == 's') {
                game.hShoot(shootDirectionChoice);
            }
                
        mtx.unlock();
        Sleep(250);
    } while (!game.getGameOver());

    
}

void gameTimerThread_Function(Game &game) {
    thread::id this_id = this_thread::get_id();
    
    do {
        mtx.lock();
            game.updateMap();
            game.updateMoveRocket();
            game.removeOutofMapRockets();
            game.updateMap();
            game.printMap();
        mtx.unlock();
        Sleep(100);
    } while (!game.getGameOver());
    game.printWinner();
}

void tThread_Function(Game& game) {
    char moveDirectionChoice,  moveOrShootChoice;
    char possibleChoice[8] = { '4','6' };
    char possibleMoveorShoot[2] = { 'm', 's' };
    srand(time(NULL));
    do {
        // decide to move or shoot
        moveDirectionChoice = possibleChoice[rand() % 2];
        moveOrShootChoice = possibleMoveorShoot[rand() % 2];

        mtx.lock();
        if (moveOrShootChoice == 'm')
            game.tMove(moveDirectionChoice);
        else if (moveOrShootChoice == 's')
            game.tShoot();
        mtx.unlock();
        Sleep(250);
    } while (!game.getGameOver());

}

void rThread_Function(Game& game) {
    char moveDirectionChoice, moveOrShootChoice;
    char possibleChoice[8] = { '8','2' };
    char possibleMoveorShoot[2] = { 'm', 's' };
    srand(time(NULL));
    do {
        // decide to move or shoot
        moveDirectionChoice = possibleChoice[rand() % 2];
        moveOrShootChoice = possibleMoveorShoot[rand() % 2];

        mtx.lock();
        if (moveOrShootChoice == 'm')
            game.rMove(moveDirectionChoice);
        else if (moveOrShootChoice == 's')
            game.rShoot();
        mtx.unlock();
        Sleep(250);
    } while (!game.getGameOver());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    // create and initial the game
    Game game;
    game.init(13, 13);
    game.restart();
    game.updateMap();
    game.printMap();

    // create threads and pass the reference of the game to each threads
    thread thread1(hThread_Function, ref(game));
    thread thread2(gameTimerThread_Function, ref(game));
    thread thread3(tThread_Function, ref(game));
    thread thread4(rThread_Function, ref(game));

    // wait for each thread to finished
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    cout << "All thread finished!" << endl;
}
