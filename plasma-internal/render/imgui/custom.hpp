#pragma once
#include <string>
#include "imgui_tricks.hpp"

inline int tab = 0;
inline int subtab = 0;
inline float content_anim = 0.f;

namespace custom {
    bool tab( const char* icon, const char* label, bool selected );
    bool subtab( const char* label, bool selected );
    void begin_child( const char* name, ImVec2 size );
    void end_child( );
    bool collapse_button( bool collapsed );
    void ImRotateStart( );
    ImVec2 ImRotationCenter( );
    void ImRotateEnd( float rad, ImVec2 center = ImRotationCenter( ) );
}


