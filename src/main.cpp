#include "PlanetSystem.hpp"
#include <ctime>
#include <cmath>
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

const float FPS = 60;
const float DT  = 1/FPS;

const float START_MASS = 100;
const float MAX_MASS   = 10e4;
const float VEL_SCALAR = 0.5;

const int INITIAL_WIDTH  = 640,
          INITIAL_HEIGHT = 480;

Vec2f randomVec(float maxMag) {
    float angle = std::rand()/(float)RAND_MAX*2*M_PI;
    Vec2f norm = {std::cos(angle),std::sin(angle)};
    return std::rand()/(float)RAND_MAX*maxMag*norm;
}

void makeVelLine(const Planet& planet,sf::VertexArray& line) {
    line[0].position = sf::Vector2f(planet.loc[0],planet.loc[1]);
    line[1].position = sf::Vector2f(line[0].position.x+planet.vel[0]/VEL_SCALAR,line[0].position.y+planet.vel[1]/VEL_SCALAR);
}

int main() {
    PlanetSystem system;
    sf::RenderWindow window(sf::VideoMode(INITIAL_WIDTH,INITIAL_HEIGHT),"Gravity");

    Planet planetToAdd(START_MASS,Vec2f::ZERO,Vec2f::ZERO);
    planetToAdd.drawUnfilled = true;
    sf::VertexArray velLine;
    velLine.resize(2);
    velLine.setPrimitiveType(sf::Lines);
    velLine[0].color = velLine[1].color = sf::Color::Blue;
    sf::VertexArray velLineTmp;
    velLineTmp.resize(2);
    velLineTmp.setPrimitiveType(sf::Lines);
    velLineTmp[0].color = velLineTmp[1].color = sf::Color::Blue;

    sf::View camera(sf::FloatRect(0,0,INITIAL_WIDTH,INITIAL_HEIGHT));

    sf::Vector2f prevMouseLoc;
    bool movingCamera = false;

    bool running = true;

    window.setFramerateLimit(FPS);
    while(window.isOpen()) {
        sf::Event event;
        while(window.pollEvent(event)) {
            switch(event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::Resized:
                camera.setSize(event.size.width,event.size.height);
                break;
            case sf::Event::MouseWheelMoved:
                {
                    float newRadius = planetToAdd.radius()+event.mouseWheel.delta;
                    planetToAdd.mass = std::min(MAX_MASS,std::exp(newRadius));
                }
                break;
            case sf::Event::MouseButtonPressed:
                if(event.mouseButton.button == sf::Mouse::Middle) {
                    system.clear();
                }
                break;
            case sf::Event::MouseButtonReleased:
                if(event.mouseButton.button == sf::Mouse::Left) {
                    system.addPlanet(planetToAdd);
                }
            case sf::Event::KeyPressed:
                if(event.key.code == sf::Keyboard::Space) {
                    system.setDrawingTrails(!system.drawingTrails());
                } else if(event.key.code == sf::Keyboard::P) {
                    running = !running;
                }
            default:
                break;
            }
        }
        if(sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            sf::Vector2f mouseLoc = sf::Vector2f(sf::Mouse::getPosition(window));
            if(!movingCamera) {
                movingCamera = true;
            } else {
                camera.move(prevMouseLoc-mouseLoc);
            }
            prevMouseLoc = mouseLoc;
        } else {
            movingCamera = false;
        }

        sf::Vector2f worldOffset = camera.getCenter()-camera.getSize()/2.0f;

        bool drawVelLine = false;
        if(!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mouseLoc = sf::Mouse::getPosition(window);
            planetToAdd.loc[0] = worldOffset.x+mouseLoc.x;
            planetToAdd.loc[1] = worldOffset.y+mouseLoc.y;
            velLine[0].position = worldOffset+sf::Vector2f(mouseLoc);
        } else {
            drawVelLine = true;
            velLine[1].position = worldOffset
                                  +sf::Vector2f(sf::Mouse::getPosition(window));
            planetToAdd.vel[0] = velLine[1].position.x - velLine[0].position.x;
            planetToAdd.vel[1] = velLine[1].position.y - velLine[0].position.y;
            planetToAdd.vel *= VEL_SCALAR;
        }
        if(running) {
            system.tick(DT);
        }
        window.clear(sf::Color::Black);

        window.setView(camera);

        window.draw(system);

        window.draw(planetToAdd);
        if(drawVelLine) {
            window.draw(velLine);
        }

        if(!running) {
            for(auto& planet:system._planets) {
                makeVelLine(planet,velLineTmp);
                window.draw(velLineTmp);
            }
        }

        window.display();
    }
}
