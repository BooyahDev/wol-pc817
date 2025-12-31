# ESP32 HTTP Server Simulator for macOS ARM64
FROM --platform=linux/arm64 ubuntu:22.04

# タイムゾーン設定
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Tokyo

# 必要なパッケージのインストール
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    curl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# 作業ディレクトリ
WORKDIR /app

# ソースファイルをコピー
COPY simulator/ /app/
COPY shared/ /app/shared/
COPY src/config.h /app/src/config.h

# ビルド
RUN g++ -std=c++17 -o esp32_simulator \
    -I./shared -I./src \
    esp32_simulator.cpp \
    -lpthread

# ポート80を公開
EXPOSE 80

# シミュレータを実行
CMD ["./esp32_simulator"]
