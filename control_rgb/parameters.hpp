// constants
const int PIN_BRIGHT = 15;
const int PIN_RED = 14;
const int PIN_GREEN = 12;
const int PIN_BLUE = 13;

const int global_delay = 20;
const int N = 255;

// handler variables
bool ON = false;
int pattern = 0;
int len_pattern_list = 0;
String jsonString;

int step = 0;
int brightness = 0;
bool ascending = true;
bool done_pattern = true;
bool done_delay = false;

// pattern variables
int global_red = 0;
int global_green = 0;
int global_blue = 0;
int static_delay = 0;
int transition_in_delay = 0;
int transition_off_delay = 0;
int sum_delay = 0;