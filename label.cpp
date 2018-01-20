#include "label.hpp"

Label::Label() {
    this->background = 0;
    this->len = 0;
}


Label::Label(uint16_t background) {
    this->background = background;
}

void Label::setPosition(uint16_t x, uint16_t y) {
    drawLabel(this->text, this->len, this->x, this->y, this->size, this->background);
    drawLabel(this->text, this->len, x, y, this->size, this->color);

    this->x = x;
    this->y = y;
}

void Label::clear() {
    drawLabel(this->text, this->len, this->x, this->y, this->size, this->background);

    this->len = 0;
}

void Label::setColor(uint16_t color) {
    drawLabel(this->text, this->len, this->x, this->y, this->size, color);

    this->color = color;
}

void Label::setText(char* text, uint8_t len) {
        uint8_t nlen = len > this->len ? len : this->len;
        char write[nlen];
        char* del = this->text;
        for (uint8_t c = 0; c < nlen; c++) {
            if (text[c] == this->text[c]) {
                write[c]=' ';
                del[c] = ' ';
            }
        }

        drawLabel(del, this->len, this->x, this->y, this->size, this->background);
        drawLabel(write, nlen, this->x, this->y, this->size, this->color);

    for(uint8_t c=0; c<len; c++) {
        this->text[c] = text[c];
    }
    this->len = len;
}


void Label::setNumber(uint8_t num) {
    char buffer[4];
    int c=0;
    if(num < 0) {
        buffer[0] = '-';
        c = 1;
    }

    for(; c<4; c++) {
        buffer[3-c] = (char)((num%10)+'0');
        num /= 10;
    }

    this->setText(buffer, 4);
}

void Label::forceText(char* text, uint8_t len) {
    drawLabel(this->text, this->len, this->x, this->y, this->size, this->background);
    drawLabel(text, len, this->x, this->y, this->size, this->color);

    for(int c=0; c<len; c++) {
        this->text[c] = text[c];
    }
    this->len = len;
}

void Label::setSize(uint8_t size) {
    drawLabel(this->text, this->len, this->x, this->y, this->size, this->background);
    drawLabel(this->text, this->len, this->x, this->y, size, this->color);

    this->size = size;
}

void Label::update() {
    drawLabel(this->text, this->len, this->x, this->y, this->size, this->background);
    drawLabel(this->text, this->len, this->x, this->y, this->size, this->color);
}