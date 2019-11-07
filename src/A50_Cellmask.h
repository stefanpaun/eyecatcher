class Cellmask {

  public:
    Cellmask(byte h, byte s, byte b){
      _h = h;
      _s = s;
      _b = b;
    }
    byte getHue() {return _h;}
    byte getSaturation() {return _s;}
    byte getBright() {return _b;}

  private:
    byte _h;
    byte _s;
    byte _b;

};
