// Trying to draw a 3D waffle to be used as a logo for the program ;-)
// Author: Felipe Correa da Silva Sanches <juca@members.fsf.org>
// License: Affero GPLv3 or later.

L = 30;
module hole(L){
    hull(){
        cube(L, center=true);
        translate([0,0,L]) cube(2*L, center=true);
    }
}

module body(){
hull(){
    cylinder(h=L/4, r=6*L, center=true);
    cylinder(h=L, r=5.5*L, center=true);
}
}


module holes(){
    intersection(){
        hull(){
            cylinder(h=L/4+1, r=5.2*L, center=true);
            cylinder(h=L+1, r=5.2*L, center=true);
        }
        for (a=[0:3]){
            rotate(90*a){
                for (b=[0,180]){
                    for (i=[1:3]){
                        for (j=[1:3]){
                            translate([-0.8*L, -0.8*L])
                            translate([L*2*i, L*2*j])
                            rotate([0, b]){
                                translate([0, 0, 2*L/3])
                                hole(L);
                            }
                        }
                    }
                }
            }
        }
    }
}

module waffle(){
    difference(){
        body();
        holes();
    }
}

rotate([0.55*360, 0.8*360, 0])
color([0.95,0.85,0.5]) waffle();
