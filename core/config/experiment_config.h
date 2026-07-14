#ifndef EXPERIMENT_CONFIG_H
#define EXPERIMENT_CONFIG_H

#include <string>
/**
 * Global experiment configuration.
 *
 * All values have defaults identical to the current behaviour.
 * Missing XML parameters keep these values.
 */
struct ExperimentConfig{
    struct Simulation{
        unsigned int seed;
        double duration;
        double timestep;
        bool headless;
        Simulation()
        :
        seed(0),
        duration(600.0),
        timestep(0.1),
        headless(false)
        {}
    };
    struct Robots{
        unsigned int number;
        std::string distribution;
        double min_distance;
        unsigned int placement_seed;
        std::string csv;
        Robots()
        :
        number(10),
        distribution("random"),
        min_distance(1.0),
        placement_seed(1),
        csv("")
        {}
    };
    struct Mobility{
        double current_strength;
        double current_factor;
        double drag_x;
        double drag_y;
        double imu_noise;
        Mobility()
        :
        current_strength(0.0),
        current_factor(0.1),
        drag_x(-0.05),
        drag_y(-0.08),
        imu_noise(0.02)
        {}
    };
    struct Channel{
        std::string mode;
        double range;
        double noise;
        double attenuation;
        double speed_sound;
        double neighbor_timeout;
        double routing_timeout;
        Channel()
        :
        mode("DEFAULT"),
        range(50.0),
        noise(0.0),
        attenuation(0.05),
        neighbor_timeout(2.0),
        routing_timeout(3.0),
        speed_sound(1500.0)
        {}
    };
    struct Localization{
        std::string algorithm;
        Localization()
        :
        algorithm("NONE")
        {}
    };
    struct MAC{
        std::string protocol;
        double aloha_probability;
        double csma_backoff;
        double csma_random_window;
        MAC()
        :
        protocol("ALOHA"),
        aloha_probability(0.7),
        csma_backoff(0.2),
        csma_random_window(0.2)
        {}
    };
    Simulation simulation;
    Robots robots;
    Mobility mobility;
    Channel channel;
    Localization localization;
    MAC mac;
    void ResetDefaults(){
        simulation = Simulation();
        robots = Robots();
        channel = Channel();
        localization = Localization();
        mac = MAC();
        mobility = Mobility();
    }
};

#endif