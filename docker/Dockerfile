###############################
# This Dockerfile is for LC4PIM
# Version: 0.1
# use multi-stage build for smaller image size
###############################

ARG UBUNTU_Version=18.04

ARG USERNAME=lc4pim
ARG USERID=1000

FROM ubuntu:${UBUNTU_Version}

# # Install basic utilities
# # reference to https://github.com/octoml/public-tvm-docker/blob/master/Dockerfile.cpu_base
# RUN apt-get update && \
#     DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends git libgtest-dev cmake wget unzip libtinfo-dev libz-dev \
#     bzip2 p7zip libcurl4-openssl-dev g++ sudo python3-dev antlr4 vim screen && \
#     apt-get clean && \
#     rm -rf /var/lib/apt/lists/* 

# Create the user as sudoer
# refer the setting in https://stackoverflow.com/a/49848507
RUN useradd -rm -d /home/$USERNAME -s /bin/bash -g root -G sudo -u $USERID $USERNAME

USER $USERNAME
WORKDIR /home/$USERNAME

# RUN useradd 
# --create-home -s /bin/bash   
# && \
    # chown $USERNAME $CONDA_DIR -R && \
    # adduser $USERNAME sudo && \
    # echo "$USERNAME ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# USER $USERNAME
# WORKDIR /home/$USERNAME
# RUN chown $USERNAME /home/$USERNAME -R