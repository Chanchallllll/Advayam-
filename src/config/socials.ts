import { FaLinkedinIn, FaGithub } from "react-icons/fa";
import { FaXTwitter } from "react-icons/fa6";

export const SOCIAL_LINKS = {
  linkedin: "https://www.linkedin.com/company/webyfyiot/",
  github: "https://github.com/orgs/Webyfy/dashboard",
  twitter: "https://x.com/advayamide",
};

export const SOCIAL_ITEMS = [
  { name: "LinkedIn", icon: FaLinkedinIn, href: SOCIAL_LINKS.linkedin },
  { name: "GitHub", icon: FaGithub, href: SOCIAL_LINKS.github },
  { name: "X (Twitter)", icon: FaXTwitter, href: SOCIAL_LINKS.twitter },
];
