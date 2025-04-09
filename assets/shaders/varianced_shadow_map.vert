#version 450

void main() {
    switch(gl_VertexIndex) {
        case 0:
            gl_Position = vec4(1, -1, 0, 1);
            return;
        case 1:
            gl_Position = vec4(-1, -1, 0, 1);
            return;
        case 2:
            gl_Position = vec4(-1, 1, 0, 1);
            return;
        case 3:
            gl_Position = vec4(1, -1, 0, 1);
            return;
        case 4:
            gl_Position = vec4(1, 1, 0, 1);
            return;
        case 5:
            gl_Position = vec4(-1, 1, 0, 1);
            return;
    }

    gl_Position = vec4(0, 0, 0, 0);
}