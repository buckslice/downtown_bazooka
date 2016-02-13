#include "entityManager.h"
EntityManager *EntityManagerInstance;//An extern from entityManager.h

EntityManager::EntityManager(Player* player) : player(player) {
    //dudeMesh = Graphics::registerMesh();
    //projectileMesh = Graphics::registerMesh(Resources::get().solidTex);
}

// should add in support for adding single entities dynamically next
// main problem is need to redo the graphics to just create buffer with fixed MAX_ENTITIES size
// then update it each time a new entity is spawned using glBufferSubData()
// or maybe glMapBuffer() or glMapBufferRange()
void EntityManager::init(int numberOfDudes) {
    EntityManagerInstance = this;
    deleteEntities();

    //std::vector<glm::vec3> colors;
    //for (int i = 0; i < numberOfDudes; i++) {
    //    glm::vec2 rnd = Mth::randomPointInSquare(CITY_SIZE);
    //    bool elite = rand() % 50 == 0;

    //    glm::vec3 scale = glm::vec3(1.0f, 2.0f, 1.0f);
    //    glm::vec3 variance = Mth::randInsideUnitCube();
    //    variance.x = variance.z = abs(variance.x);
    //    scale += variance * .25f;
    //    if (elite) {
    //        scale *= Mth::rand01() + 2.0f;
    //    }
    //    Enemy* e = new Enemy(player->transform, glm::vec3(rnd.x, 200.0f, rnd.y), scale);

    //    if (elite) { // gold elites 
    //        e->speed = Mth::rand01() * 5.0f + 10.0f;
    //        e->jumpVel = Mth::rand01() * 10.0f + 30.0f;
    //        colors.push_back(glm::vec3(0.8f, 1.0f, 0.6f));
    //    } else {    // red grunts
    //        colors.push_back(glm::vec3(1.0f, Mth::rand01() * 0.3f, Mth::rand01() * 0.3f));
    //    }

    //    entities.push_back(e);
    //    //cols.push_back(glm::vec3(Mth::rand01(), Mth::rand01(), Mth::rand01()));
    //}
    //Graphics::setColors(dudeMesh, colors);

    //colors.clear();

    //for (int i = 0; i < MAX_PROJECTILES; i++) {
    //    colors.push_back(glm::vec3(1.0f, 0.2f, 0.0f));
    //}

    //Graphics::setColors(projectileMesh, colors);

    // init particles
    particles.clear();
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle p;
        particles.push_back(p);
    }

}

void EntityManager::update(float delta) {
    player->update(delta);

    std::vector<glm::mat4> models;
    //for (size_t i = 0, len = entities.size(); i < len; ++i) {
    //    entities[i]->update(delta);

    //    //// update dudes model
    //    //glm::mat4 model;
    //    //glm::vec3 pos = entities[i]->getTransform()->lpos;
    //    //glm::vec3 scale = entities[i]->getTransform()->scale;
    //    //pos.y += scale.y / 2.0f;
    //    //model = glm::translate(model, pos);
    //    //model = glm::scale(model, scale);
    //    models.push_back(entities[i]->getTransform()->getModelMatrix());
    //}
    //Graphics::setModels(dudeMesh, models);
    //models.clear();

    // add projectiles models
    //std::vector<Projectile>& projectiles = player->getProjectiles();
    //for (size_t i = 0, len = projectiles.size(); i < len; ++i) {
    //    PhysicsTransform* pt = projectiles[i].getTransform();

    //    //glm::mat4 model;
    //    //glm::vec3 pos = pt->getWorldPos();
    //    //glm::vec3 scale = pt->scale;
    //    //model = glm::translate(model, pos);
    //    //model = glm::scale(model, scale);
    //    models.push_back(pt->getModelMatrix());
    //}

    //Graphics::setModels(projectileMesh, models);
    //models.clear();

    //// update and build particles
    //// should probably remove this into seperate class..

    std::vector<glm::vec3> colors;
    for (size_t i = 0, len = particles.size(); i < len; ++i) {
        Particle& p = particles[i];

        p.update(delta);

        if (p.transform >= 0) {

            //glm::mat4 model;
            //PhysicsTransform* pt = p->getTransform();
            //glm::vec3 pos = pt->getPos();
            //glm::vec3 scale = pt->scale;
            //pos.y += scale.y / 2.0f;
            //model = glm::translate(model, pos);
            //model = glm::scale(model, scale);
            models.push_back(p.getTransform()->getModelMatrix());

            colors.push_back(p.getColor());
        }
    }

    // add particles to solid box stream
    Graphics::addToStream(true, models, colors);

}


void EntityManager::deleteEntities() {
    for (size_t i = 0, len = entities.size(); i < len; ++i) {
        delete entities[i];
    }
    entities.erase(entities.begin(), entities.end());
}

Particle* EntityManager::getNextParticle() {
    Particle *p = &particles[curParticle];
    curParticle = (++curParticle) % (MAX_PARTICLES - 1);
    return p;
}

void EntityManager::SpawnParticle(glm::vec3 pos, int effect, float randvel, glm::vec3 vel) {
    Particle* p = getNextParticle();
    p->effect = effect;
    p->activate();
    PhysicsTransform* pt = p->getTransform();
    pt->setPos(pos);
    pt->setScale(glm::vec3(.25f));
    pt->vel = vel + Mth::randInsideSphere(1.0f) * randvel;
    //pt->vel = vel + Mth::randInsideUnitCube() * randvel;
}
