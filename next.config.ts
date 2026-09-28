import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  async redirects() {
    const communityUrl = process.env.NEXT_PUBLIC_COMMUNITY_URL;
    if (!communityUrl) {
      return [];
    }

    return [
      {
        source: "/community",
        destination: communityUrl,
        permanent: false,
      },
      {
        source: "/community/:path*",
        destination: `${communityUrl}/:path*`,
        permanent: false,
      },
    ];
  },
};

export default nextConfig;
