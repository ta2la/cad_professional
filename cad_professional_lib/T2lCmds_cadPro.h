//
// Copyright (C) 2014 Petr Talla. [petr.talla@gmail.com]
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//		      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=============================================================================
#pragma once

// infrastructure
#include <TcCmdEngine.h>

namespace T2l {

//=============================================================================
class  Cmds_cadPro {
//=============================================================================
public:
//<CMDS>
//=============================================================================
//<REGISTRATOR>
    static bool registerCmds_()
    {
        //commands-----------------------------------------
        REGISTER_CMD( "cad_draw_line_pro",      cmd_cad_draw_line_pro,      "cad_pro" );
        REGISTER_CMD( "cad_export_dxf",         cmd_cad_export_dxf,         "cad_pro" );
        REGISTER_CMD( "cad_import_dxf",         cmd_cad_import_dxf,         "cad_pro" );
        REGISTER_CMD( "cad_draw_arc",           cmd_cad_draw_arc,           "cad_pro" );
        REGISTER_CMD( "cad_draw_arc2",          cmd_cad_draw_arc2,           "cad_pro" );

        //settings pro-------------------------------------
        REGISTER_CMD( "cad_set_line_style",     cmd_cad_set_line_style,     "cad_pro" );
        REGISTER_CMD( "cmd_cad_export_svg",     cmd_cad_export_svg,         "cad_pro" );

        REGISTER_CMD( "cmd_object_split",           cmd_object_split,           "cad" );
        REGISTER_CMD( "cmd_object_movepoints",           cmd_object_movepoints,           "cad" );

        return true;
    }
private:
//<INTERNALS>
    Cmds_cadPro();

    //commands-----------------------------------------
    CMD_FCE( cmd_cad_draw_line_pro );
    CMD_FCE( cmd_cad_draw_arc );
    CMD_FCE( cmd_cad_draw_arc2 );
    CMD_FCE( cmd_cad_export_dxf );
    CMD_FCE( cmd_cad_import_dxf );

    //settings pro-------------------------------------
    CMD_FCE( cmd_cad_set_line_style );
    CMD_FCE(cmd_cad_export_svg);

    CMD_FCE( cmd_object_split );
    CMD_FCE( cmd_object_movepoints );


};

} //namespace T2l
