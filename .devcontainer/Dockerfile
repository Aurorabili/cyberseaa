FROM debian:bookworm
USER root

# Creating a non-root user. More info: https://aka.ms/vscode-remote/containers/non-root.
ARG USERNAME=containerUser
ARG USER_UID=1000
ARG USER_GID=$USER_UID
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    #
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# [Optional] Replace APT sources to use CN mirrors
# RUN sed -i 's/deb.debian.org/ftp.cn.debian.org/g' /etc/apt/sources.list.d/debian.sources

# Install needed packages. Use a separate RUN statement to add your own dependencies.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install build-essential wget curl git netcat-openbsd unzip cmake ca-certificates apt-transport-https dpkg-dev patch libc6-dev automake autoconf \
    && apt-get autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/*

# Install Clang-17
RUN echo "deb https://apt.llvm.org/bookworm llvm-toolchain-bookworm-17 main" \
        > /etc/apt/sources.list.d/llvm.list && \
    wget -qO /etc/apt/trusted.gpg.d/llvm.asc \
        https://apt.llvm.org/llvm-snapshot.gpg.key && \
    apt-get update && export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -qqy -t llvm-toolchain-bookworm-17 clang-17 clang-tidy-17 clang-format-17 lld-17 libc++-17-dev libc++abi-17-dev


# [Optional] Set the default user. Omit if you want to keep the default as root.
USER $USERNAME

# [Optional] Uncomment this section to install additional OS packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>

# Install xmake
RUN curl -fsSL https://xmake.io/shget.text | bash
