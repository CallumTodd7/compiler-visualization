docker build -t cv_ubuntu .
docker run -it --rm -v $PWD/ubuntu_data:/home/work cv_ubuntu
