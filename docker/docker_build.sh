#!/bin/sh

# build the docker image if want suspend at the 1st build stage
# DOCKER_BUILDKIT=1 docker build --target build -t pytorch1.1_cuda9 --force-rm .

# Enable buildkit builds (linux container only)
docker_id=elliothe
repo_name=lc4pim
tag=latest
# Dockerile used for docker build
dockerfile_to_use=/Users/elliot/Documents/GitHub/LS4PIM/docker/Dockerfile

# Print arguments
echo "Dockerfile: ${dockerfile_to_use}"

DOCKER_BUILDKIT=1 docker build --tag ${docker_id}/${repo_name}:${tag} --force-rm . \
    --file "${dockerfile_to_use}" 

# prune all the intermediate images.
# docker system prune

# push to docker hub
# docker login
# docker push ${docker_id}/${repo_name}:${tag}