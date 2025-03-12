
#ifndef GAME_STATE_HPP
    #define GAME_STATE_HPP
    #include <vector>
    #include <map>
    #include <memory>

    class Entity;
    class Player;
    class Dungeon;

    class GameState {
        public:
            GameState();
            ~GameState();

            // Game Loop
            void update();

            // Dungeon Management
            void generateDungeon();
            Dungeon *getDungeon();

            // Player Management
            /* TODO: Implement Multiple Players */

            // Entity Management
            void AddEntity(Entity *entity);
            void removeEntity(Entity *entity);
            Entity *getEntity(int id);

        private:
            std::unique_ptr<Dungeon> _dungeon;
            std::unique_ptr<Player> _player;
            std::map<int, Entity*> _entities;
    };
#endif // GAME_STATE_HPP
