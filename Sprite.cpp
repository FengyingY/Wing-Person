#include "Sprite.hpp"

Sprite::Sprite(std::string path, std::string sprite_name)  {
    name = sprite_name;
    load_png(path, &size, &data, LowerLeftOrigin);

    // from game0 base code
    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &element_buffer);

    { //vertex array mapping buffer for color_texture_program:
        //ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
        glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

        //set vertex_buffer_for_color_texture_program as the current vertex array object:
        glBindVertexArray(vertex_buffer_for_color_texture_program);

        //set vertex_buffer as the source of glVertexAttribPointer() commands:
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        //set up the vertex array object to describe arrays of PongMode::Vertex:
        glVertexAttribPointer(
            color_texture_program.Position_vec4, //attribute
            3, //size
            GL_FLOAT, //type
            GL_FALSE, //normalized
            sizeof(Vertex), //stride
            (GLbyte *)0 + 0 //offset
        );
        glEnableVertexAttribArray(color_texture_program.Position_vec4);
        //[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

        glVertexAttribPointer(
            color_texture_program.Color_vec4, //attribute
            4, //size
            GL_UNSIGNED_BYTE, //type
            GL_TRUE, //normalized
            sizeof(Vertex), //stride
            (GLbyte *)0 + 4*3 //offset
        );
        glEnableVertexAttribArray(color_texture_program.Color_vec4);

        glVertexAttribPointer(
            color_texture_program.TexCoord_vec2, //attribute
            2, //size
            GL_FLOAT, //type
            GL_FALSE, //normalized
            sizeof(Vertex), //stride
            (GLbyte *)0 + 4*3 + 4*1 //offset
        );
        glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

        //done referring to vertex_buffer, so unbind it:
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //done setting up vertex array object, so unbind it:
        glBindVertexArray(0);

        GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
    }

    {
        //assign texture:
        
        //ask OpenGL to fill white_tex with the name of an unused texture object:
        glGenTextures(1, &png_tex);

        //bind that texture object as a GL_TEXTURE_2D-type texture:
        glBindTexture(GL_TEXTURE_2D, png_tex);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

        //set filtering and wrapping parameters:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        //since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
        glGenerateMipmap(GL_TEXTURE_2D);

        //Okay, texture uploaded, can unbind it:
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}


Sprite::~Sprite() {
    //----- free OpenGL resources -----
    glDeleteBuffers(1, &vertex_buffer);
    vertex_buffer = 0;

    glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
    vertex_buffer_for_color_texture_program = 0;

    glDeleteTextures(1, &png_tex);
    png_tex = 0;
}

void Sprite::draw(glm::vec2 center_pos, glm::vec2 drawable_size, float scale, float direction) {
    //vertices will be accumulated into this list and then uploaded+drawn at the end of this function:
    std::vector< Vertex > vertices;

    // inline helper function for rectangle drawing:
    auto draw_rectangle = [&vertices](glm::vec2 const &center, glm::vec2 const &radius, glm::u8vec4 const &color) {
        //draw rectangle as two CCW-oriented triangles:
        vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.0f, 0.0f));
        vertices.emplace_back(glm::vec3(center.x+radius.x, center.y-radius.y, 0.0f), color, glm::vec2(1.0f, 0.0f));
        vertices.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(1.0f, 1.0f));

        vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.0f, 0.0f));
        vertices.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(1.0f, 1.0f));
        vertices.emplace_back(glm::vec3(center.x-radius.x, center.y+radius.y, 0.0f), color, glm::vec2(0.0f, 1.0f));
    };

    draw_rectangle(center_pos, glm::vec2(size.x * scale * direction, size.y * scale), glm::u8vec4(0xff, 0xff, 0xff, 0xff));
    
    
    //use alpha blending:
    // glEnable(GL_BLEND); // This is causing issues. Disabled for the time being
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //don't use the depth test:
    glDisable(GL_DEPTH_TEST);

    //upload vertices to vertex_buffer:
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); //set vertex_buffer as current
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW); //upload vertices array
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(color_texture_program.program);

    glm::mat4 projection = glm::ortho(0.0f, (float)drawable_size.x, 0.0f, (float)drawable_size.y);
    glUniformMatrix4fv(color_texture_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vertex_buffer_for_color_texture_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, png_tex);
    glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    
}