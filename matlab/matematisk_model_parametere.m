
R=9.73;
L=1.27*10^-3;
Kt=27.8*10^-3;
Kb=(1/344)*((2*pi)/60);
Tau_m=31.9*10^-3;
%original J 
%J=25.3*10^-7;
J=25.3*10^-7;
TL=0;
Tau_e=L/R;
b=J/Tau_m;
vinkel_has = 4090;
dreiemoment = vinkel_has/121
s= tf('s');
h= Kt/(((L*s+R)*(J*s+b))+ Kt*Kb) 
figure(1)
step(h)
grid on
