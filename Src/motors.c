/**
 * @file    motors.c
 * @brief   PWM 4 canaux sur TIM3 et logique de déplacement du robot.
 *
 *   CH1 = PA6 | CH2 = PA7 | CH3 = PB0 | CH4 = PB1   (AF2)
 *   PSC = 15, ARR = 999  ->  f = 16 MHz / (16 x 1000) = 1 kHz
 *
 *   Pont en H : CH1/CH2 commandent le côté A (avant/arrière),
 *               CH3/CH4 commandent le côté B (avant/arrière).
 */
#include "board.h"
#include "motors.h"

static volatile EtatMoteur etat_moteur = ETAT_STOP;
static volatile uint16_t   motor_speed = MOTOR_SPEED_DEFAULT;

void TIM3_PWM_Init(void)
{
    RCC->AHB1ENR |= (1 << 0);   /* GPIOA */
    RCC->AHB1ENR |= (1 << 1);   /* GPIOB */
    RCC->APB1ENR |= (1 << 1);   /* TIM3  */

    /* PA6 = CH1, PA7 = CH2 : AF2 */
    GPIOA->MODER  &= ~((3 << 12) | (3 << 14));
    GPIOA->MODER  |=  ((2 << 12) | (2 << 14));
    GPIOA->AFR[0] &= ~((0xF << 24) | (0xF << 28));
    GPIOA->AFR[0] |=  ((2 << 24) | (2 << 28));

    /* PB0 = CH3, PB1 = CH4 : AF2 */
    GPIOB->MODER  &= ~((3 << 0) | (3 << 2));
    GPIOB->MODER  |=  ((2 << 0) | (2 << 2));
    GPIOB->AFR[0] &= ~((0xF << 0) | (0xF << 4));
    GPIOB->AFR[0] |=  ((2 << 0) | (2 << 4));

    TIM3->PSC = 15;
    TIM3->ARR = PWM_MAX;

    /* PWM mode 1 (OCxM = 110) + préchargement OCxPE sur les 4 canaux */
    TIM3->CCMR1 &= ~((0xFF << 0) | (0xFF << 8));
    TIM3->CCMR1 |=  (6 << 4) | (1 << 3) | (6 << 12) | (1 << 11);
    TIM3->CCMR2 &= ~((0xFF << 0) | (0xFF << 8));
    TIM3->CCMR2 |=  (6 << 4) | (1 << 3) | (6 << 12) | (1 << 11);

    TIM3->CCER |= (1 << 0) | (1 << 4) | (1 << 8) | (1 << 12);   /* CC1E..CC4E */
    TIM3->CCR1 = TIM3->CCR2 = TIM3->CCR3 = TIM3->CCR4 = 0;

    TIM3->CR1 |= (1 << 7);   /* ARPE */
    TIM3->EGR |= (1 << 0);   /* UG : charge PSC/ARR */
    TIM3->CR1 |= (1 << 0);   /* CEN */
}

/* Écrit les rapports cycliques correspondant à l'état courant. */
static void appliquer_vitesse(void)
{
    const uint16_t inner = (uint16_t)(motor_speed * TURN_INNER_PERCENT / 100);

    switch (etat_moteur)
    {
        case ETAT_AVANT:
            TIM3->CCR1 = motor_speed; TIM3->CCR2 = 0;
            TIM3->CCR3 = motor_speed; TIM3->CCR4 = 0;
            break;

        case ETAT_ARRIERE:
            TIM3->CCR1 = 0; TIM3->CCR2 = motor_speed;
            TIM3->CCR3 = 0; TIM3->CCR4 = motor_speed;
            break;

        case ETAT_GAUCHE:   /* virage progressif : côté B ralenti */
            TIM3->CCR1 = motor_speed; TIM3->CCR2 = 0;
            TIM3->CCR3 = inner;       TIM3->CCR4 = 0;
            break;

        case ETAT_DROITE:   /* virage progressif : côté A ralenti */
            TIM3->CCR1 = inner;       TIM3->CCR2 = 0;
            TIM3->CCR3 = motor_speed; TIM3->CCR4 = 0;
            break;

        default:            /* ETAT_STOP */
            TIM3->CCR1 = TIM3->CCR2 = TIM3->CCR3 = TIM3->CCR4 = 0;
            break;
    }
}

void motor_forward(void)  { etat_moteur = ETAT_AVANT;   appliquer_vitesse(); }
void motor_backward(void) { etat_moteur = ETAT_ARRIERE; appliquer_vitesse(); }
void motor_left(void)     { etat_moteur = ETAT_GAUCHE;  appliquer_vitesse(); }
void motor_right(void)    { etat_moteur = ETAT_DROITE;  appliquer_vitesse(); }
void motor_stop(void)     { etat_moteur = ETAT_STOP;    appliquer_vitesse(); }

void motor_set_speed(uint16_t speed)
{
    motor_speed = (speed > PWM_MAX) ? PWM_MAX : speed;
    appliquer_vitesse();
}

void cmd_ROBOT(char cmd)
{
    switch (cmd)
    {
        case 'F': motor_forward();  break;
        case 'B': motor_backward(); break;
        case 'L': motor_left();     break;
        case 'R': motor_right();    break;
        case 'S': motor_stop();     break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            motor_set_speed((uint16_t)((cmd - '0') * 111));   /* 0 .. 999 */
            break;

        case 'q':
            motor_set_speed(PWM_MAX);
            break;

        default:
            break;
    }
}
