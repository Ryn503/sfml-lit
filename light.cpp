#include "light.hpp"
#include <cmath>
#include <algorithm>
#include <string>

namespace lit
{
    Light::Light() = default;

    Light::Light(std::vector<Line>& _lines)
    {
        for (auto& line : _lines)
        {
            lines.push_back(line);
        }

        findVertices();
    }

    bool Light::init()
    {
        if (!shader.isAvailable()) return false;

        const std::string light_frag =                                                        \
            "#version 120\n                                                                 " \
            "                                                                               " \
            "uniform sampler2D texture;                                                     " \
            "uniform vec4 lightColor;                                                       " \
            "uniform float lightEnergy;                                                     " \
            "uniform highp vec2 lightOrigin;                                                " \
            "uniform highp float lightIntensity;                                            " \
            "                                                                               " \
            "float getDistance(in vec2 from, in vec2 to)                                    " \
            "{                                                                              " \
            "   return sqrt(pow(from.x - to.x, 2.0) + pow(from.y - to.y, 2.0));             " \
            "}                                                                              " \
            "                                                                               " \
            "void main()                                                                    " \
            "{                                                                              " \
            "   highp vec2 pixel = gl_FragCoord.xy;                                         " \
            "   float dist = getDistance(pixel, lightOrigin) * lightIntensity / 100.0;      " \
            "   vec4 color = lightColor;                                                    " \
            "   color += texture2D(texture, pixel - 4.0) * 0.0162162162;                    " \
            "   color += texture2D(texture, pixel - 3.0) * 0.0540540541;                    " \
            "   color += texture2D(texture, pixel - 2.0) * 0.1216216216;                    " \
            "   color += texture2D(texture, pixel) * 0.1945945946;                          " \
            "   color += texture2D(texture, pixel) * 0.2270270270;                          " \
            "   color += texture2D(texture, pixel) * 0.1945945946;                          " \
            "   color += texture2D(texture, pixel + 2.0) * 0.1216216216;                    " \
            "   color += texture2D(texture, pixel + 3.0) * 0.0540540541;                    " \
            "   color += texture2D(texture, pixel + 4.0) * 0.0162162162;                    " \
            "   color = mix(vec4(color.rgb, lightEnergy), vec4(0.0, 0.0, 0.0, 0.0), dist);  " \
            "                                                                               " \
            "   gl_FragColor = color;                                                       " \
            "}                                                                              " ;

        if (!shader.loadFromMemory(light_frag, sf::Shader::Fragment)) return false;

        shader.setUniform("texture", sf::Shader::CurrentTexture);
        shader.setUniform("lightOrigin", sf::Glsl::Vec2(position));
        shader.setUniform("lightIntensity", intensity);
        shader.setUniform("lightEnergy", energy);
        shader.setUniform("lightColor", sf::Glsl::Vec4(color));
        
        return true;
    }

    void Light::draw(sf::RenderWindow& window, bool debug_lines, const sf::Color& lines_color)
    {
        if (vertices.size() == 0) return;
    
        sf::Vector2i coords = window.mapCoordsToPixel({position.x, position.y - window.getView().getSize().y});
        shader.setUniform("lightOrigin", sf::Glsl::Vec2(coords.x, -coords.y));
        window.draw(&vertex[0], vertex.size(), sf::TriangleFan, &shader);

        if (debug_lines)
        {
            sf::Vertex line_to_draw[2];

            auto draw_line = [&](const Line& line) -> void
                {
                    line_to_draw[0].position = line.start;
                    line_to_draw[1].position = line.end;
                    line_to_draw[0].color = lines_color;
                    line_to_draw[1].color = lines_color;

                    window.draw(line_to_draw, 2, sf::Lines);
                };

            for (auto& line : lines)
            {
                draw_line(line);
            }
        }
    }

    void Light::clear()
    {
        lines.clear();
        findVertices();
    }

    void Light::addLine(const Line& line)
    {
        lines.push_back(line);
        findVertices();
    }

    void Light::removeLine(int index)
    {
        vertices.erase(vertices.begin() + index);
        findVertices();
    }

    Line Light::getLine(int index) const
    {
        return lines[index];
    }

    int Light::getLinesCount() const
    {
        return lines.size();
    }

    void Light::setPosition(const sf::Vector2f& _position)
    {
        position = _position;

        std::vector<float> angles;
        for (auto& vertice : vertices)
        {
            float angle = atan2f(vertice.y - position.y, vertice.x - position.x);
            angles.push_back(angle - 0.001);
            angles.push_back(angle - 0.0001);
            angles.push_back(angle - 0.00001);
            angles.push_back(angle);
            angles.push_back(angle + 0.00001);
            angles.push_back(angle + 0.0001);
            angles.push_back(angle + 0.001);
        }

        std::vector<Intersection> intersections;
        for (auto& angle : angles)
        {
            sf::Vector2f distance {
                cosf(angle), sinf(angle)
            };
            Line ray {
                position, {position.x + distance.x, position.y + distance.y}
            };

            Intersection closest_intersect;
            for (auto& line : lines)
            {
                auto intersect = getIntersection(ray, line);
                if (!intersect.valid) continue;
                if (!closest_intersect.valid || intersect.param < closest_intersect.param)
                {
                    closest_intersect = intersect;
                }
            }

            if (!closest_intersect.valid) continue;
            closest_intersect.angle = angle;
            intersections.push_back(closest_intersect);
        }

        std::sort(intersections.begin(), intersections.end(), [](Intersection a, Intersection b) -> bool
            {
                return a.angle < b.angle;
            });

        if (intersections.size() == 0) return;

        vertex.resize(intersections.size() + 1);

        vertex[0].position = position;
        vertex[0].color = color;

        for (int i = 0; i < intersections.size(); i++)
        {
            vertex[i + 1].position = intersections[i].position;
            vertex[i + 1].color = color;
        }

        vertex[intersections.size()].position = intersections[0].position;
        vertex[intersections.size()].color = color;
    }

    sf::Vector2f Light::getPosition() const
    {
        return position;
    }

    void Light::setColor(const sf::Color& _color)
    {
        color = _color;
        color.a = 255;
        shader.setUniform("lightColor", sf::Glsl::Vec4(color));
    }

    sf::Color Light::getColor() const
    {
        return color;
    }

    void Light::setIntensity(float _intensity)
    {
        intensity = _intensity;
        shader.setUniform("lightIntensity", intensity);
    }

    float Light::getIntensity() const
    {
        return intensity;
    }

    void Light::setEnergy(float _energy)
    {
        energy = _energy;
        shader.setUniform("lightEnergy", energy);
    }

    float Light::getEnergy() const
    {
        return energy;
    }

    void Light::findVertices()
    {
        std::vector<sf::Vector2f> points;

        for (auto& line : lines)
        {
            points.push_back(line.start);
            points.push_back(line.end);
        }

        for (const auto& point : points)
        {
            if (!(std::find(vertices.begin(), vertices.end(), point) != vertices.end()))
            {
                vertices.push_back(point);
            }
        }
    }

    Light::Intersection Light::getIntersection(const Line& ray, const Line& segment)
    {
        sf::Vector2f ray_distance {ray.end.x - ray.start.x, ray.end.y - ray.start.y};
        sf::Vector2f segment_distance {segment.end.x - segment.start.x, segment.end.y - segment.start.y};
        Intersection intersect;

        if (ray_distance.x * segment_distance.y == ray_distance.y * segment_distance.x)
        {
            return intersect;
        }

        float param2 = (ray_distance.x * (segment.start.y - ray.start.y) + ray_distance.y * (ray.start.x - segment.start.x)) / (segment_distance.x * ray_distance.y - segment_distance.y * ray_distance.x);
        float param1 = (segment_distance.x * (ray.start.y - segment.start.y) + segment_distance.y * (segment.start.x - ray.start.x)) / (ray_distance.x * segment_distance.y - ray_distance.y * segment_distance.x);

        if (param1 < 0.0f || param2 < 0.0f || param2 > 1.0f)
        {
            return intersect;
        }

        intersect.position.x = ray.start.x + ray_distance.x * param1;
        intersect.position.y = ray.start.y + ray_distance.y * param1;
        intersect.param = param1;
        intersect.valid = true;

        return intersect;
    }
}