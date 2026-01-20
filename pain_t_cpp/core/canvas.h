#ifndef CANVAS_H
#define CANVAS_H

#include <cstdint>
#include <vector>
#include <string>

class Canvas{
public:
    Canvas(int width, int height);
    ~Canvas() = default;

    int getWidth() const {return width;}
    int getHeight() const {return height;}

    uint32_t getPixel(int x, int y) const;
    void setPixel(int x, int y, uint32_t color);
    void clear(uint32_t color);

    void 
    setSize(int w, int h) {
        width = w;
        height = h;
        pixels.resize(w * h, 0xFFFFFFFF);
    }

    void 
    setPixelsBlock(int x, int y, int w, int h, const std::vector<uint32_t>& data){
        for (int iy = 0; iy < h; ++iy){
            int cy = y + iy;
            if (cy < 0 || cy >= height) continue;
            for (int ix = 0; ix < w; ++ix){
                int cx = x + ix;
                if (cx < 0 || cx >= width) continue;
                pixels[cy * width + cx] = data[iy * w + ix];
            }
        }
    }
    
    bool savePNG(const std::string& path) const;

    const std::vector<uint32_t>& getPixels() const{return pixels;}

    std::vector<uint32_t>& getPixels(){return pixels;}

private:
    int width;
    int height;
    std::vector<uint32_t> pixels;
};


#endif
