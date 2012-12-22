/*
 * adf7012.c
 *
 *  Created on: 2012. 12. 19.
 *      Author: OSSI
 */

#include "adf7012.h"

static volatile uint8_t vco_bias;
static volatile uint8_t vco_adj;

struct {
    struct {
        uint16_t  frequency_error_correction;
        uint8_t r_divider;
        uint8_t crystal_doubler;
        uint8_t crystal_oscillator_disable;
        uint8_t clock_out_divider;
        uint8_t vco_adjust;
        uint8_t output_divider;
    } r0;

    struct {
        unsigned int  fractional_n;
        uint8_t integer_n;
        uint8_t prescaler;
    } r1;

    struct {
    	uint8_t mod_control;
    	uint8_t gook;
    	uint8_t power_amplifier_level;
    	uint16_t  modulation_deviation;
        uint8_t gfsk_modulation_control;
        uint8_t index_counter;
    } r2;

    struct {
    	uint8_t pll_enable;
        uint8_t pa_enable;
        uint8_t clkout_enable;
        uint8_t data_invert;
        uint8_t charge_pump_current;
        uint8_t bleed_up;
        uint8_t bleed_down;
        uint8_t vco_disable;
        uint8_t muxout;
        uint8_t ld_precision;
        uint8_t vco_bias;
        uint8_t pa_bias;
        uint8_t pll_test_mode;
        uint8_t sd_test_mode;
    } r3;
} adf_config;

void ADF7012_setup(void)
{
	// port init
	IO_DIRECTION(TXDATA, OUTPUT);
	//IO_DIRECTION(TXCLK, INPUT); // TODO: for GOOK modulation
	IO_DIRECTION(SCLK,OUTPUT);
	IO_DIRECTION(SDATA,OUTPUT);
	IO_DIRECTION(SLE,OUTPUT);
	IO_DIRECTION(CE,OUTPUT);
	IO_DIRECTION(PAON,OUTPUT);
	IO_DIRECTION(MUXOUT,INPUT);

	// default pin state
	IO_SET(PAON, LOW);
	IO_SET(TXDATA, LOW);
	IO_SET(SCLK, LOW);
	IO_SET(SDATA, LOW);
	IO_SET(SLE, LOW);
	IO_SET(CE, LOW);
}

void ADF7012_init(void)
{
	// default pin state
	IO_SET(PAON, LOW);
	IO_SET(TXDATA, LOW);
	IO_SET(SCLK, LOW);
	IO_SET(SDATA, LOW);
	IO_SET(SLE, LOW);
	IO_SET(CE, LOW);

	// configure timer interrupt
}

void byte_write(unsigned char _register)
{
	volatile int i;

	for(i = 7; i >= 0; i--)
	{
		IO_SET(SCLK, LOW);
		if(_register & (1<<i)) {
			IO_SET(SDATA, HIGH);
		}
		else {
			IO_SET(SDATA, LOW);
		}
		__delay_cycles(5);
		IO_SET(SCLK, HIGH);
		__delay_cycles(15);
	}
	IO_SET(SCLK, LOW);
}

void ADF7012_regWrite(uint32_t registers)
{
	volatile int i;
	unsigned char _register;

	IO_SET(SCLK, LOW);
	IO_SET(SDATA, LOW);
	IO_SET(SLE, HIGH);
	__delay_cycles(10);
	IO_SET(SLE, LOW);
	__delay_cycles(10);
	//for(i=0;i<sizeof(registers);i++)
	for(i=0;i<sizeof(registers);i++)
	{
		_register = (registers >> 24-(i*8)) & 0xff;
		byte_write(_register);
	}
    // SLE
	__delay_cycles(10);
	IO_SET(SLE, HIGH);

	IO_SET(SDATA, LOW);                       // SDATA low
	//IO_SET(SLE, LOW);
	__delay_cycles(30);

}


void ADF7012_init_register_zero(void) {
    adf_config.r0.frequency_error_correction = 1879;               // Don't bother for now...
    adf_config.r0.r_divider = 5;          						// Whatever works best for 2m, 1.25m and 70 cm ham bands
    adf_config.r0.crystal_doubler = 0;                          // Who would want that? Lower f_pfd means finer channel steps.
    adf_config.r0.crystal_oscillator_disable = 0;               // Disable internal crystal oscillator because we have an external VCXO
    adf_config.r0.clock_out_divider = 1;                        // Don't bother for now...
    adf_config.r0.vco_adjust = 0;                               // Don't bother for now... (Will be automatically adjusted until PLL lock is achieved)
    adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_4;     // Pre-set div 4 for 2m. Will be changed according tx frequency on the fly
}

void ADF7012_init_register_one(void) {
    adf_config.r1.integer_n = 37;                              // Pre-set for 144.390 MHz APRS. Will be changed according tx frequency on the fly
    adf_config.r1.fractional_n = 4074;                          // Pre-set for 144.390 MHz APRS. Will be changed according tx frequency on the fly
    adf_config.r1.prescaler = 0;                			   // 8/9 requires an integer_n > 91; 4/5 only requires integer_n > 31
}

void ADF7012_init_register_two(void) {
    adf_config.r2.mod_control = ADF_MODULATION_OOK;             // For AFSK the modulation is done through the external VCXO we don't want any FM generated by the ADF7012 itself
    adf_config.r2.gook = 0;                                     // Whatever... This might give us a nicer swing in phase maybe...
    adf_config.r2.power_amplifier_level = 0;                    // Start PA power from 0. Output −20dBm at 0x0, and 13 dBm at 0x7E at 868 MHz

    adf_config.r2.modulation_deviation = 0;                    // 16 is about half maximum amplitude @ ASK.
    adf_config.r2.gfsk_modulation_control = 0;                  // Don't bother for now...
    adf_config.r2.index_counter = 0;                            // Don't bother for now...
}

void ADF7012_init_register_three(void) {
    adf_config.r3.pll_enable = 0;                               // Switch off PLL (will be switched on after Ureg is checked and confirmed ok)
    adf_config.r3.pa_enable = 0;                                // Switch off PA  (will be switched on when PLL lock is confirmed)
    adf_config.r3.clkout_enable = 0;                            // No clock output needed at the moment
    adf_config.r3.data_invert = 0;                              // Results in a TX signal when TXDATA input is low
    adf_config.r3.charge_pump_current = ADF_CP_CURRENT_1_5; //ADF_CP_CURRENT_0_3; //ADF_CP_CURRENT_2_1;     // 2.1 mA. This is the maximum
    adf_config.r3.bleed_up = 0;                                 // Don't worry, be happy...
    adf_config.r3.bleed_down = 0;                               // Dito
    adf_config.r3.vco_disable = 0;                              // VCO is on

    adf_config.r3.muxout = ADF_MUXOUT_REG_READY;                // Lights up the green LED if the ADF7012 is properly powered (changes to lock detection in a later stage)

    adf_config.r3.ld_precision = ADF_LD_PRECISION_5_CYCLES;     // What the heck? It is recommended that LDP be set to 1; 0 is more relaxed
    adf_config.r3.vco_bias = 6;                                 // In 0.5 mA steps; Default 6 means 3 mA; Maximum (15) is 8 mA
    adf_config.r3.pa_bias = 4;                                  // In 1 mA steps; Default 4 means 8 mA; Minimum (0) is 5 mA; Maximum (7) is 12 mA (Datasheet says uA which is bullshit)
    adf_config.r3.pll_test_mode = 0;
    adf_config.r3.sd_test_mode = 0;
}

void ADF7012_init_all_registers(void)
{
	ADF7012_init_register_zero();
	ADF7012_init_register_one();
	ADF7012_init_register_two();
	ADF7012_init_register_three();
}

void adf7012_write_register_zero(void) {

    uint32_t reg =
        (0) |
        ((uint32_t)(adf_config.r0.frequency_error_correction & 0x7FF) << 2) |
        ((uint32_t)(adf_config.r0.r_divider & 0xF ) << 13) |
        ((uint32_t)(adf_config.r0.crystal_doubler & 0x1 ) << 17) |
        ((uint32_t)(adf_config.r0.crystal_oscillator_disable & 0x1 ) << 18) |
        ((uint32_t)(adf_config.r0.clock_out_divider & 0xF ) << 19) |
        ((uint32_t)(adf_config.r0.vco_adjust & 0x3 ) << 23) |
        ((uint32_t)(adf_config.r0.output_divider & 0x3 ) << 25);

    ADF7012_regWrite(reg);
}

void adf7012_write_register_one(void) {
    uint32_t reg =
        (1) |
        ((uint32_t)(adf_config.r1.fractional_n & 0xFFF) << 2) |
        ((uint32_t)(adf_config.r1.integer_n & 0xFF ) << 14) |
        ((uint32_t)(adf_config.r1.prescaler & 0x1 ) << 22);

    ADF7012_regWrite(reg);
}

void adf7012_write_register_two(void) {
    uint32_t reg =
        (2) |
        ((uint32_t)(adf_config.r2.mod_control & 0x3 ) << 2) |
        ((uint32_t)(adf_config.r2.gook & 0x1 ) << 4) |
        ((uint32_t)(adf_config.r2.power_amplifier_level & 0x3F ) << 5) |
        ((uint32_t)(adf_config.r2.modulation_deviation & 0x1FF) << 11) |
        ((uint32_t)(adf_config.r2.gfsk_modulation_control & 0x7 ) << 20) |
        ((uint32_t)(adf_config.r2.index_counter & 0x3 ) << 23);

    ADF7012_regWrite(reg);
}

void adf7012_write_register_three(void) {
    uint32_t reg =
        (3) |
        ((uint32_t)(adf_config.r3.pll_enable & 0x1 ) << 2) |
        ((uint32_t)(adf_config.r3.pa_enable & 0x1 ) << 3) |
        ((uint32_t)(adf_config.r3.clkout_enable & 0x1 ) << 4) |
        ((uint32_t)(adf_config.r3.data_invert & 0x1 ) << 5) |
        ((uint32_t)(adf_config.r3.charge_pump_current & 0x3 ) << 6) |
        ((uint32_t)(adf_config.r3.bleed_up & 0x1 ) << 8) |
        ((uint32_t)(adf_config.r3.bleed_down & 0x1 ) << 9) |
        ((uint32_t)(adf_config.r3.vco_disable & 0x1 ) << 10) |
        ((uint32_t)(adf_config.r3.muxout & 0xF ) << 11) |
        ((uint32_t)(adf_config.r3.ld_precision & 0x1 ) << 15) |
        ((uint32_t)(adf_config.r3.vco_bias & 0xF ) << 16) |
        ((uint32_t)(adf_config.r3.pa_bias & 0x7 ) << 20) |
        ((uint32_t)(adf_config.r3.pll_test_mode & 0x1F ) << 23) |
        ((uint32_t)(adf_config.r3.sd_test_mode & 0xF ) << 28);

    ADF7012_regWrite(reg);
}


void ADF7012_enable(void)
{
	IO_SET(CE, HIGH);
	__delay_cycles(10);
}

void ADF7012_disable(void)
{
	IO_SET(CE, LOW);
	__delay_cycles(10);
}

void ADF7012_write_all_registers(void)
{
	adf7012_write_register_zero();
	adf7012_write_register_one();
	adf7012_write_register_two();
	adf7012_write_register_three();
}

uint8_t ADF7012_isLocked(void)
{
	if ((P1IN & BIT1) > 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t ADF7012_isManyLocked(void)
{
	volatile uint16_t cnt;
	volatile uint16_t i;
	cnt = 0;

	for(i = 0; i < 1000 ; i++) // sample PLL  lock muxout for 1000 times
	{
		delay_ms(1); // total check time is 1000 * 1 ms = 1 sec
		if ((P1IN & BIT1) > 0)
		{
			cnt ++;
		}
		else
		{
			if(cnt > 0) // prevent overflow
			{
				cnt --;
			}
		}
	}

	if (cnt > 500) // if more than 500 times lock -> PLL Locked
	{
		printf("Lock Count %u\r\n",cnt);
		return 1;

	}
	else // if less than 500 times lock -> PLL not locked
	{
		printf("Lock Count %u\r\n",cnt);
		return 0;
	}
}

void ADF7012_set_VCO(uint8_t bias, uint8_t adj)
{
	adf_config.r3.vco_bias = vco_bias = bias;
	adf_config.r0.vco_adjust = vco_adj = adj;

}

void ADF7012_set_PLL(uint8_t val)
{
	adf_config.r3.pll_enable = val;
}

void ADF7012_set_PA(uint8_t val)
{
	adf_config.r3.pa_enable = val;
}

void ADF7012_set_PALevel(uint8_t val)
{
	min_max(0,63,val);
	adf_config.r2.power_amplifier_level = val;

}


void ADF7012_set_muxout(uint8_t val)
{
	//ADF_MUXOUT_DIGITAL_LOCK = 4
	adf_config.r3.muxout = val;
}

uint8_t ADF7012_lock(void)
{
    // fiddle around with bias and adjust capacity until the vco locks

	uint8_t adj = adf_config.r0.vco_adjust; // use default start values from setup
	uint8_t bias = adf_config.r3.vco_bias;  // or the updated ones that worked last time
	volatile uint16_t i;

    adf_config.r3.pll_enable = 1;
    adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
    ADF7012_enable();
    ADF7012_write_all_registers();
    delay_ms(50);
    ADF7012_isLocked();

    while(!ADF7012_isLocked()) {

//        Serial.print("VCO not in lock. Trying adj: ");
//        Serial.print(adj);
//        Serial.print(" and bias: ");
//        Serial.println(bias);
        adf_config.r0.vco_adjust = adj;
        adf_config.r3.vco_bias = bias;
        adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
        ADF7012_enable();
        ADF7012_write_all_registers();
        delay_ms(1);
        if(++bias == 14) {
            bias = 1;
            if(++adj == 4) {
                //Serial.println("Couldn't achieve PLL lock :( ");
                // Using best guess defaults:
                adf_config.r0.vco_adjust = 0;
                adf_config.r3.vco_bias = 5;

                return 0;
            }

        }

        delay_ms(1);
    }
    printf("PLL locked!\r\n");
    printf("VCO Bias: %u\r\n",bias);
    printf("VCO adjust: %u\r\n",adj);

    ADF7012_set_VCO(bias,adj);
    return 1;
}

void ADF7012_findLock(void)
{
    // fiddle around with bias and adjust capacity until the vco locks

	uint8_t adj = 0;
	uint8_t bias = 0;
	volatile uint16_t i;

    adf_config.r3.pll_enable = 1;
    adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
    ADF7012_enable();
    ADF7012_write_all_registers();
    delay_ms(50);
    ADF7012_isLocked();

    for (bias = 0 ; bias <15 ; bias++)
    {
    	for (adj = 0; adj<5;adj++)
    	{
            adf_config.r0.vco_adjust = adj;
            adf_config.r3.vco_bias = bias;
            adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
            adf_config.r3.pa_enable = 0;
            ADF7012_enable();
            ADF7012_write_all_registers();
            delay_ms(300);

    		if(ADF7012_isManyLocked())
    		{
    			//resetExtWatchdogTimer();
				printf("PLL locked!\r\n");
				printf("VCO Bias: %u\r\n",bias);
				printf("VCO adjust: %u\r\n",adj);
				printf("\r\n");
				adf_config.r3.pa_enable = 1; //ADF PA On
				ADF7012_enable();
				ADF7012_write_all_registers();
				IO_SET(TXDATA,HIGH);
				delay_ms(3000);
				IO_SET(TXDATA,LOW);
	            adf_config.r3.pa_enable = 0;
	            ADF7012_enable();
	            ADF7012_write_all_registers();
    		}
    		else
    		{
    			printf("Not locked!\r\n");
				printf("VCO Bias: %u\r\n",bias);
				printf("VCO adjust: %u\r\n",adj);
				printf("\r\n");
    		}
    	}

    }

}

void ADF7012_findLockOnce(uint8_t bias,uint8_t adj)
{
	volatile uint16_t i;

	adf_config.r3.pll_enable = 1;
	adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
	ADF7012_enable();
	ADF7012_write_all_registers();
	delay_ms(50);
	ADF7012_isLocked();

	adf_config.r0.vco_adjust = adj;
	adf_config.r3.vco_bias = bias;
	adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
	adf_config.r3.pa_enable = 0;
	ADF7012_enable();
	ADF7012_write_all_registers();
	delay_ms(300);

	if(ADF7012_isManyLocked())
	{
		//resetExtWatchdogTimer();
		printf("PLL locked!\r\n");
		printf("VCO Bias: %u\r\n",bias);
		printf("VCO adjust: %u\r\n",adj);
		printf("\r\n");
		adf_config.r3.pa_enable = 1; //ADF PA On
		ADF7012_enable();
		ADF7012_write_all_registers();
		IO_SET(TXDATA,HIGH);
		delay_ms(3000);
		IO_SET(TXDATA,LOW);
		adf_config.r3.pa_enable = 0;
		ADF7012_enable();
		ADF7012_write_all_registers();
	}
	else
	{
		printf("Not locked!\r\n");
		printf("VCO Bias: %u\r\n",bias);
		printf("VCO adjust: %u\r\n",adj);
		printf("\r\n");
	}
}


void ADF7012_OOK(uint8_t val)
{
	volatile uint8_t pa_level;

	if(val)
	{

		ADF7012_init_all_registers();
		ADF7012_set_PLL(1);
		ADF7012_set_muxout(ADF_MUXOUT_DIGITAL_LOCK);
		ADF7012_set_VCO(vco_bias, vco_adj); // get VCO set values when PLL is locked from ADF7012_lock();
		ADF7012_set_PALevel(63);
		ADF7012_set_PA(1);
		ADF7012_enable();
		ADF7012_write_all_registers();
		//TODO: increase PA level gradually and write update register value while TXDATA is HIGH from 0 to certain PA level
		IO_SET(TXDATA,HIGH);
//		for (pa_level = 0; pa_level < 64; pa_level++) // 63 is max PA level
//		{
//			ADF7012_set_PALevel(pa_level);
//			adf7012_write_register_two();
//		}

	}
	else
	{
		IO_SET(TXDATA,LOW);
//		ADF7012_init_all_registers();
//		for (pa_level = 63; pa_level > 0 ; pa_level--) // 63 is max PA level
//		{
//			ADF7012_set_PALevel(pa_level);
//			adf7012_write_register_two();
//		}
		ADF7012_disable();
	}
}

