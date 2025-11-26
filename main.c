#include <bitwise.h>
#include <stm32f4xx.h>
#include "stdio.h"
#include "uart_utils.h"
#include "state_functions.h"
#include "pi_controller.h"
#include "converter_model.h"

extern uint32_t SystemCoreClock; // Siäisen kellon määritys

// --- GLOBAALIT MUUTTUJAT ---

// Tilamuuttuja (1, 2 tai 3)
static volatile int state = 1;

// Referenssijännite (muokataan UARTilla tilassa 3). Voidaan myös vaihtaa lähtemään nollasta!!!
static volatile float reference_voltage = 1.0f;

// Ajastimen vaihemuuttuja 5s eston hallintaan:
// 0 = Odotetaan debouncea (n. 30ms) (napin häiriön estoon)
// 1 = Odotetaan napin lukitusaikaa (5000ms) (semafori lukitus)
static volatile int timer_phase = 0; // Asetetaan alkuun, nollaan jotta ensin voidaan poistaa napin häiriö.

// Haetaan säädettävät parametrit state_functions.c:stä
extern volatile float param_Kp;
extern volatile float param_Ki;

// --- INTERRUPTIT ---

// 1. NAPPI (PC13) - Käynnistää prosessin. Tässä kohtaa ensin interruptin avulla poiustetaa napin häiriö
void __attribute__((interrupt("IRQ"))) EXTI15_10_IRQHandler()
{
    // Estä uudet interruptit napista heti
    bit_clr(EXTI->IMR, 13);

    // Aseta ajastin lyhyelle debounce-viiveelle (n. 30ms)
    TIM3->ARR = 30;
    TIM3->CNT = 0;

    // Aseta vaihe: debounce. TÄMÄN AVULLA VOIDAAN MYÖHEMMIN SIIRTYÄ 5 sekunnin LUKITUKSEEN
    timer_phase = 0;

    // Käynnistä ajastin
    TIM3->CR1 |= TIM_CR1_CEN; // KÄYNNISTETÄÄN AJASTIN 30 MS AJAKSI

    // Nollaa EXTI-lippu, eli asetetaan nappi uudelleen käyttöön.
    bit_set(EXTI->PR, 13);
}

// 2. AJASTIN - Hoitaa tilan vaihdon ja 5s lukituksen
void __attribute__((interrupt("IRQ"))) TIM3_IRQHandler()
{
    // Nollaa ajastimen keskeytyslippu
    TIM3->SR &= ~TIM_SR_UIF;

    if (timer_phase == 0) // KUN NAPIN MAHDOLLINEN HÄIRIÖ ON SELVITETTY, JATKETAAN TILAN VAIHTOA JA LUKITAAN SEMAFORI
    {
        // --- VAIHE 0: DEBOUNCE VALMIS ---

        // Pysäytä ajastin asetusten ajaksi
        TIM3->CR1 &= ~TIM_CR1_CEN;

        // Vaihda tilaa
        state++;
        if (state > 3) {
            state = 1; // TILAT MENEVÄT 1 -> 2 -> 3 --> 1...
        }

        // Suorita tilakohtaiset toiminnot
        switch (state)
        {
            case 1:
                handle_state_1(); // Omasta mielestä nää tilojen tai toimintojen nimet vois vaihtaa
                break;
            case 2:
                handle_state_2();
                break;
            case 3:
                handle_state_3();
                break;
        }

        //printf("State: %d. Button locked for 5s.\r\n", state); // Tällä voidaan ilmoittaa konsoliin lukitus. EI PAKOLLINEN!!!

        // --- VALMISTELE VAIHE 1 (5s LUKITUS) ---
        // Oletus: Ajastimen kello on 1 kHz (1ms tick), joten 5000 = 5s
        TIM3->ARR = 5000;
        TIM3->CNT = 0;

        timer_phase = 1; // Asetataan ajastin muuttuja tilaan 1, jotta tauon jälkeen voidaan vapauttaa nappi taas

        TIM3->CR1 |= TIM_CR1_CEN; // Käynnistä ajastin uudelleen. TÄSSÄ KOHTAA AJASTIIN LASKEE 5 SEKUNTIIN
    }
    else if (timer_phase == 1)
    {
        // --- VAIHE 1: 5 SEKUNTIA KULUNUT ---

        // Pysäytä ajastin kokonaan
        TIM3->CR1 &= ~TIM_CR1_CEN;

        // Salli napin painallus uudelleen. TÄMÄN AVULLA POISTETAAN SEMAFORIN LUKITUS
        bit_set(EXTI->PR, 13); // Varmistusnollaus lipulle
        bit_set(EXTI->IMR, 13);

        //printf("Button unlocked.\r\n"); // EI OLE PAKOLLINEN.

        // Nollaa vaihe
        timer_phase = 0; // Tämän tilan asettaminen varmistaa, että napin häiriön poiston jälkeen tulee 5 sekunnin tauko. Tämä määritys on tehty varmistuksena.
    }
}


// 3. Ajastin 4 (TIM4) - säätöloopin ajastin

void __attribute__((interrupt("IRQ"))) TIM4_IRQHandler(){
	TIM4->SR &= ~TIM_SR_UIF; // Nollataan TIM4 keskeytyslippu
	if (state == 3) // Säätölooppi ajetaan vain modulaatiotilassa 3
	    {
	        static float y_act = 0.0f; // Muuttuja mallin nykyiselle jännitteelle. Lähdetään siis nollasta voltista liikenteelle
	        float u_control;           // Säätimen laskema ohjausarvo

	        // 1. Kutsutaan PI-säädintä (y_ref, y_act, Ki, Kp)
	        u_control = PI(reference_voltage, y_act, param_Ki, param_Kp);

	        // 2. Rajoitetaan ohjausarvo välille 0.0 - 3.3V (Hardware limits)
	        if (u_control > 3.3f) u_control = 3.3f;
	        if (u_control < 0.0f) u_control = 0.0f;

	        // 3. Syötetään tämän hetkinen jännitteen arvo, eli ohjausarvo mallille ja saadaan uusi "mittaus"
	        y_act = runConverterModel(u_control);

	        // 4. Päivitetään PWM-arvo LEDin kirkkautta varten. Tämä on harjoituksesta 5
	        // Lasketaan suhde: (jännite / 3.3V) * 3999 (ajastimen maksimi)
	        uint32_t pwm_value = (uint32_t)((y_act / 3.3f) * 3999.0f);

	        // Varmistetaan ettei arvo ylitä rekisterin maksimia
	        if (pwm_value > 3999) pwm_value = 3999;

	        // Kirjoitetaan arvo TIM2 kanavan 1 vertailurekisteriin
	        TIM2->CCR1 = pwm_value;
	    }

}

// 3. UART (USART2) - Jännitteen säätö 'w' ja 's' UART-interrupteilla. TÄMÄN AVULLA VOIDAAN SÄÄTÄÄ REFERENSSIJÄNNITETTÄ MODULAATIOTILASSA SUORAAN, ILMAN KONFIGURAATIOON MENEMISTÄ 'W' JA 'S' NÄPPÄIMILLÄ
void __attribute__((interrupt("IRQ"))) USART2_IRQHandler()
{
    // Tarkista onko dataa liikkumassa (RXNE lippu)
    if (USART2->SR & USART_SR_RXNE)
    {
        char cmd = USART2->DR; // Lue merkki

        // Toimitaan vain tilassa 3 (Modulaatio)
        if (state == 3)
        {
            if (cmd == 'w')
            {
                // 'w' = UP
                reference_voltage += 0.1f;
                if (reference_voltage > 3.3f) reference_voltage = 3.3f; // RAJOITUS MAKSIMIJÄNNITTEELLE. Voidaan myös asettaa esim 5 volttiin

                // Tässä kohtaa päivitys PI-säätimelle,
                // PIController_SetReference(reference_voltage);

                printf("Voltage UP: %.2f V\r\n", reference_voltage); // EI PAKOLLINEN. ILMOITTAA KÄYTTÄJÄLLE JÄNNITETASON
            }
            else if (cmd == 's')
            {
                // 's' = DOWN
                reference_voltage -= 0.1f;
                if (reference_voltage < 0.0f) reference_voltage = 0.0f; // RAJOITUS MAKSIMIJÄNNITTEELLE

                // PIController_SetReference(reference_voltage);

                printf("Voltage DOWN: %.2f V\r\n", reference_voltage);// EI PAKOLLINEN. ILMOITTAA KÄYTTÄJÄLLE JÄNNITETASON
            }
        }
    }
}


// --- PÄÄOHJELMA ---

int main()
{
    // --- 1. KELLOJEN ALUSTUS ---
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN | RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Tärkeä EXTI:lle

    // --- 2. UART ALUSTUS ---
    uart_init();

    // Salli UART-vastaanottokeskeytys
    USART2->CR1 |= USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART2_IRQn);

    // --- 3. TIM2 ALUSTUS (PWM LEDille) ---
    // Konfiguroidaan 1 kHz PWM (Exercise 5 mukaisesti)
    TIM2->PSC   = 3;                // Prescaler: 16MHz / (3+1) = 4MHz
    TIM2->ARR   = 3999;             // Auto-reload: 4MHz / (3999+1) = 1kHz
    bits_val(TIM2->CCMR1, 4, 1, 7); // Asetetaan kanava 1 PWM mode 2:een
    TIM2->CCER |= TIM_CCER_CC1E;    // Kytketään kanavan 1 lähtö päälle

    // --- 4. NAPPI (PC13) ALUSTUS ---
    bits_val(GPIOC->MODER, 2, 13, 0); // Input
    bits_val(GPIOC->PUPDR, 2, 13, 1); // Pull-up

    // --- 5. KESKEYTYSTEN KONFIGUROINTI ---

    // route external interrupt 13 to GPIOC
    SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13;	// clear EXTI13
    SYSCFG->EXTICR[3] |=  SYSCFG_EXTICR4_EXTI13_PC;	// set EXTI13 to GPIOC

    // configure external interrupt 13
    bit_set(EXTI->IMR, 13);  // enable interrupt
    bit_clr(EXTI->RTSR, 13); // disable rising edge
    bit_set(EXTI->FTSR, 13); // enable falling edge

    // CONFIGURE TIMER 3
    TIM3->PSC = 16000 - 1;
    TIM3->ARR = 30;      	// Alkuarvo (debounce)
    TIM3->DIER |= TIM_DIER_UIE; // enable interrupt
    TIM3->CR1 |= TIM_CR1_URS;   // set counter overflow as only event source (VOIDAAN ASETTAA AJASTIMIA, JOTKA AJAN TÄYTTYESSÄ TEKEVÄT JOTAIN)

    // --- 6. TIM4 ALUSTUS (Säätölooppi 1 kHz) ---
    TIM4->PSC = (SystemCoreClock / 1000) - 1;          // Prescaler: 16MHz / 16000 = 1 kHz kello
    TIM4->ARR = 1;                  // Auto-reload: 1 ms välein keskeytys
    TIM4->DIER |= TIM_DIER_UIE;     // Sallitaan Update Interrupt
    TIM4->CR1 |= TIM_CR1_CEN;       // Käynnistetään ajastin heti (pyörii taustalla)

    // ENABLE INTERRUPTS
    NVIC_EnableIRQ(EXTI15_10_IRQn);	// enable BUTTON IRQ
    NVIC_EnableIRQ(TIM3_IRQn);		// enable Debounce ja 5 s delay TIMER IRQ
    NVIC_EnableIRQ(TIM4_IRQn);      // Control loop timer

    // --- 7. ALKUTILAN ASETUS ---
    state = 1;
    handle_state_1();
    printf("System started. State: 1\r\n");

    // --- 7. PÄÄSILMUKKA ---
    while (1)
    {
        // Prosessori nukkuu ja herää vain keskeytyksistä. Tällä komennolla prossu ei pyöri täydellä kellotaajuudella vaan odottaa että tulee keskeytykset.
        __WFI();
    }
}
