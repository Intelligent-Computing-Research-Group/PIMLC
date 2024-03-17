###############################
# This Dockerfile is for LC4PIM
# Version: 0.1
# use multi-stage build for smaller image size
###############################

ARG UBUNTU_Version=18.04
ARG USERNAME=docker
ARG USERID=1000

FROM ubuntu:UBUNTU_Version  

# Create the user as sudoer
RUN useradd --create-home -s /bin/bash --no-user-group -u $USERID $USERNAME && \
    chown $USERNAME $CONDA_DIR -R && \
    adduser $USERNAME sudo && \
    echo "$USERNAME ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers && \
    echo "export PYTHONPATH=$PROJECT_HOME:${PYTHONPATH}"

USER $USERNAME
WORKDIR /home/$USERNAME
RUN chown $USERNAME /home/$USERNAME -R
RUN mkdir /home/$USERNAME/repo

# COPY --chown=1000 --from=build /opt/conda/. $CONDA_DIR