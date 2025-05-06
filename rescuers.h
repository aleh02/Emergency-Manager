typedef enum {
    IDLE, EN_ROUTE_TO_SCENE, ON_SCENE, RETURNING_TO_BASE
} rescuer_status_t;

typedef struct {
    char* rescuer_type_name;
    int speed;
    int x, y;
    rescuer_type_t;
} rescuer_type_t;

typedef struct {
    int id;
    int x, y;
    rescuer_type_t* rescuer;
    rescuer_status_t status;
} rescuer_digital_twin_t;
