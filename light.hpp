#ifndef LIT_LIGHT_HPP
#define LIT_LIGHT_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

namespace lit
{
    /*
        \brief The value of PI in single-precision.
    */
    const float PI = 3.141592741f;

    /*
        \brief The line struct used in light.
    */
    struct Line
    {
        sf::Vector2f start;
        sf::Vector2f end;
    };

    /*
        \brief The light class.
    */
    class Light
    {
        private:
            struct Intersection
            {
                sf::Vector2f position;
                float param;
                float angle;
                bool valid = false;
            };

            void findVertices();
            Intersection getIntersection(const Line& ray, const Line& segment);

        private:
            std::vector<sf::Vertex> vertex;
            std::vector<Line> lines;
            std::vector<sf::Vector2f> vertices;
            sf::Shader shader;
            sf::Vector2f position {0.0f, 0.0f};
            sf::Color color {255, 255, 255};
            float intensity {1.0f};
            float energy {1.0f};

        public:
            /*
                \brief Create a new light.
            */
            Light();

            /*
               \brief Create a new light with pre defined lines.

               \param lines the vector of lines to use as occlusion.
            */
            Light(std::vector<Line>& lines);

            /*
                \brief Start the light shader.

                You always have to call this function after create a light,
                so that light have the shader effect.

                \return Return true if success otherwise false.
            */
            bool init();
            /*
                \brief Draw the light.

                \param window the RenderWindow to draw to.
                \param debug_lines to enable debug lines.
                \param lines_color the debug lines colors.
            */
            void draw(sf::RenderWindow& window, bool debug_lines = false, const sf::Color& lines_color = {0, 255, 255});

            /*
                \brief Clear all lines.

                \warning You need to add lines to draw, ortherwise this won't draw anything.
            */
            void clear();
            /*
                \brief Add a new line.

                \param line the line to append.
            */
            void addLine(const Line& line);
            /*
                \brief Remove a line by index.

                \param index the position of the line.
            */
            void removeLine(int index);
            /*
                \brief Get a line by index.

                \param index the position of the line.

                \return Return the line.
            */
            Line getLine(int index) const;
            /*
                \brief Get the total of lines in the light.

                \return Return the total of lines.
            */
            int getLinesCount() const;

            /*
                \brief Set the light position.

                \param position the position to set.
            */
            void setPosition(const sf::Vector2f& position);
            /*
                \brief Get the light position.

                \return Return the position of the light.
            */
            sf::Vector2f getPosition() const;

            /*
                \brief Set the light color.

                \param color the color to change, alpha is ignored.
            */
            void setColor(const sf::Color& color);
            /*
                \brief Get the light color.

                \return Return the RGB color.
            */
            sf::Color getColor() const;

            /*
                \brief Set the light intensity.

                You use this to control the max distance of the light,
                less value increase distance and more value decrease distance.
                
                \param intensity the intensity of the light.

                \warning You need to init the light first.
            */
            void setIntensity(float intensity);
            /*
                \brief Get the light intensity.

                \return Return the intensity of the light.
            */
            float getIntensity() const;

            /*
                \brief Set the light energy.

                You use this to control the brit brightness of the light.

                \param energy the energy of the light.

                \warning You need to init the light first.
            */
            void setEnergy(float energy);
            /*
                \brief Get the light energy.

                \return Return the energy of the light.
            */
            float getEnergy() const;
            
    };
}

#endif