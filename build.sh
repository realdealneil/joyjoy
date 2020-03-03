g++ -std=c++11 `pkg-config --cflags libevdev glib-2.0` -I. joyjoy_app.cpp `pkg-config --libs libevdev glib-2.0` -o joyjoy
