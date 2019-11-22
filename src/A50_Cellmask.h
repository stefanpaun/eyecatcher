class Cellmask {

  public:
    Cellmask(int h, byte s, byte b){
      _h = h;
      _s = s;
      _b = b;
    }
    int getHue() {return _h;}
    byte getSaturation() {return _s;}
    byte getBright() {return _b;}

  private:
    int _h;
    byte _s;
    byte _b;

};
