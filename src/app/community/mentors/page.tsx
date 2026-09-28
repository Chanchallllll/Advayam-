import CommunityMembersPage from "@/components/community/CommunityMembersPage";
import { mentors } from "@/data/community";

export default function MentorsPage() {
  return (
    <CommunityMembersPage
      title="All Mentors"
      subtitle="Meet the mentors guiding the Advayam community with real-world experience."
      people={mentors}
    />
  );
}