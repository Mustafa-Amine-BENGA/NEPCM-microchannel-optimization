#include "udf.h"

#define Cp_NEPCM        1830
#define Cp_water        4179
#define rho_NEPCM       1033
#define rho_water       997.1
#define k_NEPCM         0.2
#define k_water         0.613
#define mu_water        797e-6
#define h_sf            152600.0
#define T_mr            0.5
#define pi              3.14159265359
#define T_fusion        301.15

real phi   = 0.04;


DEFINE_ADJUST(update_params, d)
{
#if !RP_NODE
    if (RP_Variable_Exists_P("phi")) 
    {
        phi = RP_Get_Real("phi");
    }
#endif

    /* Synchronisation parallèle : Le HOST distribue les valeurs aux autres cœurs */
    host_to_node_real_1(phi);
}


DEFINE_PROPERTY(rho_mixture_NEPCM,c,t)
{

    real rho_mixture = (1 - phi)*rho_water + phi*rho_NEPCM ;
    return rho_mixture;
}

DEFINE_PROPERTY(k_mixture_NEPCM,c,t)
{
    
    real k_mixture = (2*k_water + k_NEPCM + 2*phi*(k_NEPCM - k_water)) / (2*k_water + k_NEPCM - phi*(k_NEPCM - k_water))*k_water ;
    return k_mixture;
}

DEFINE_PROPERTY(mu_mixture_NEPCM,c,t)
{
    real mu_mixture = mu_water * (1 + 2.5*phi + 6.5*phi*phi) ;
    return mu_mixture ;
}


DEFINE_SPECIFIC_HEAT(Cp_NEPCM_mixture, T, Tref, h, yi)
{
    
    
    real  Cp_mixture , h_mixture ;
    real T1 = T_fusion - (T_mr/2.0); 
    real T2 = T_fusion + (T_mr/2.0);
    real rho_mixture = (1 - phi)*rho_water + phi*rho_NEPCM ;
    real omega = (phi * rho_NEPCM)/rho_mixture ;
     

    if (T < T1)
    {
        Cp_mixture = omega * Cp_NEPCM + (1 - omega)*Cp_water ;
        h_mixture = Cp_mixture * (T - Tref);
    }
    else if (T > T2)
    {
        Cp_mixture = omega * Cp_NEPCM + (1 - omega)*Cp_water ;
        h_mixture = Cp_mixture * (T - Tref) + omega*(h_sf - Cp_NEPCM*T_mr) ;
    }
    else
    {
        real amplitude = (pi/2.0) * (h_sf/T_mr - Cp_NEPCM);
        real pulsation = pi / T_mr;
        Cp_mixture = omega*(Cp_NEPCM + ((pi/2.0) * (h_sf/T_mr - Cp_NEPCM) * sin(pi*(T - T1)/T_mr))) + (1-omega) * Cp_water;
        h_mixture = omega * (Cp_NEPCM * (T - Tref) + (amplitude / pulsation) * (1.0 - cos(pulsation * (T - T1)))) + ((1 - omega)*Cp_water*(T - Tref)) ;
    }

    *h = h_mixture ;
    
    
    return Cp_mixture;
}


