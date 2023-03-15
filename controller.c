// ---------- Máquina de estados -----------

#include "IO.c"

#undef DEBUG

// Tipos de dados

// Estados da máquina
typedef enum
{
	PARADO,	 // 0
	OPERAR,	 // 1
	A_PARAR, // 2
} modos;

typedef enum
{
	TAPETES_ON,	 // 0
	ANDAR_VERDE, // 1
	EMPURRAR_P,	 // 2
	RETRAIR_P,	 // 3
	ANDAR_AZUL,	 // 4
	ANDAR_AZUL2, // 5
	BUFFER,		 // 6
	PARAR_T,	 // 7
} separacao;

typedef enum
{
	WAIT_OFF, // 0
	WAIT_ON,  // 1
} luz_do_a_parar;

// timer block typedef
typedef struct
{
	bool on = false;
	uint64_t time = 0;
} timerBlock;

// Funções
void initME();
void update_timers();
void start_timer(timerBlockt *t);
void stop_timer(timerBlock *t);

// Estado atual da máquina
modos currentState_modos = PARADO;
separacao currentState_separacao = Parar_T;
luz_do_a_parar currentState_luz = WAIT_OFF;

// Tempo de ciclo
uint64_t scan_time = 1000; // 1 segundo

// timers
uint64_t start_time = 0, end_time = 0, cycle_time = 0;
timerBlock timer1;
timerBlock timer2;

// variáveis de flancos
bool FE_START = false; // falling edge do botao start
int startPreviousState = 0;

bool RE_STOP = false; // rising edge do botao stop
int stopPreviousState = 0;

bool RE_SPR = false;
int sprPreviousState = 0;

bool RE_SPE = false;
int spePreviousState = 0;

bool FE_STR = false;
int strPreviousState = 0;

bool FE_ST2 = false;
int st2PreviousState = 0;

bool FE_ST3 = false;
int st3PreviousState = 0;

// Atualiza timers
void update_timers()
{
	// Calcula o tempo de ciclo
	end_time = get_time();
	if (start_time == 0)
		cycle_time = 0;
	else
		cycle_time = end_time - start_time; // o fim do ciclo atual é o inicio do próximo start_time = end_time;

	// Atualiza temporizadores
	if (timer1.on)
		timer1.time = timer1.time + cycle_time;
	if (timer2.on)
		timer2.time = timer2.time + cycle_time;
}

void start_timer(timerBlock *t)
{
	t->on = true;
	t->time = 0;
}

void stop_timer(timerBlock *t)
{
	t->on = false;
	t->time = 0;
	// t->time pode ser comentado
	// se for importante guardar
	// o valor do temporizador
}

// Inicializa a ME
void init_ME()
{
	LSTART = 0;
	LSTOP = 0;
	LWAIT = 0;
	E1 = 0;
	E2 = 0;
	T1A = 0;
	T2A = 0;
	T3A = 0;
	T4A = 0;
	PE1 = 0;
	PR1 = 0;
	PE2 = 0;
	PR2 = 0;
	uint16_t AZUIS = 0;
	uint16_t VERDES = 0;
}

// Código principal
int main()
{
	init_ME(); // Inicialização da ME

	// Ciclo de execução
	while (1)
	{

#ifdef DEBUG
		printf("\n*** Inicio do Ciclo ***\n");
#endif

		// Leitura das entradas
		read_inputs();

		// detetecao de flancos
		FE_START = (startPreviousState && !START) ? true : false;
		startPreviousState = START;
		RE_STOP = (!stopPreviousState && STOP) ? true : false;
		stopPreviousState = STOP;
		RE_SPR = (!sprPreviousState && SPR1) ? true : false;
		sprPreviousState = SPR1;
		RE_SPE = (!spePreviousState && SPE1) ? true : false;
		spePreviousState = SPE1;
		FE_STR = (strPreviousState && !STR1) ? true : false;
		strPreviousState = STR1;
		FE_ST2 = (st2PreviousState && !ST2) ? true : false;
		st2PreviousState = ST2;
		FE_ST3 = (st2PreviousState && !ST2) ? true : false;
		st2PreviousState = ST2;

		update_timers();

		// Transição entre estados MODOS
		switch (currentState_modos)
		{

		case PARADO:
			// Testa transição Parado -> Operar
			if (FE_START == true)
				// Próximo estado
				currentState_modos = OPERAR;

			break;

		case OPERAR:
			// Testa transição OPERAR -> A_PARAR
			if (RE_STOP == true)
				currentState_modos = A_PARAR;

			break;

		case A_PARAR:
			// Testa transição A_PARAR -> PARADO
			if (currentState_separacao == PARAR_T)
				currentState_modos = PARADO;

			break;

		default:
			break;
		}

		// Transição entre estados separacao
		//TODO é necessário refazer a implementação do timer
		switch (currentState_separacao)
		{
		case TAPETES_ON:
			start_timer(timer1);
			if (SV1 == 4)
				currentState_separacao = ANDAR_VERDE;
				stop_timer(timer1);
			else if (SV == 1)
				currentState_separacao = ANDAR_AZUL;
				stop_timer(timer1);
			else if (A_PARAR == true && timerTapetesOn10s >=)
				currentState_separacao = BUFFER;
				stop_timer(timer1);

			break;

		case ANDAR_VERDE:
			if (FE_STR == true)
				currentState_separacao = EMPURRAR_P;
			break;

		case EMPURRAR_P:
			if (RE_SPE == true)
				currentState_separacao = RETRAIR_P;
			break;

		case RETRAIR_P:
			if (RE_SPR == true)
				currentState_separacao = TAPETES_ON;
			break;

		default:
			break;
		}

		// Transição entre estados LUZ_DO_A_PARAR
		switch (currentState_luz)
		{
		case WAIT_OFF:
		
			break;

		case WAIT_ON: // scatman

			break;

		default:
			break;
		}

		// Atualiza saídas

		// Saídas booleanas
		FILL = (currentState == Encher);
		DRAIN = (currentState == Esvaziar);

		// Escrita nas saídas
		write_outputs();

		// Aguarda pelo próximo ciclo
		sleep_abs(scan_time);

	} // end loop

} // end main
