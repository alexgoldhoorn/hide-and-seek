#ifndef PARTICLE_H
#define PARTICLE_H

/*! This class
 * 
 */
struct Particle {
    //! default constructor
    Particle();

    //! init constructor
    Particle(double x, double y, double h);

    //! x,y position 
    double x,y;
    //! heading
    double h;
    //! weight
    double w;

    //! returns Particle which would have been read
    Particle readSensor();

    
    /**
     * @brief move the particle 1 step
     *
     * @param speed the speed (movement per time step), can be noisy
     * @param headingNoise, added to the heading value, can be 0 
     *
     * @return 
     */
    bool advance(double speed, double headingNoiseStd = 0);

};

#endif
