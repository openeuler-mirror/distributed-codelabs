dnf install docker wget -y
wget https://repo.openeuler.org/openEuler-22.03-LTS-SP2/docker_img/aarch64/openEuler-docker.aarch64.tar.xz
docker load -i openEuler-docker.aarch64.tar.xz
docker build -f dockerfile -t softbus_client_image .
